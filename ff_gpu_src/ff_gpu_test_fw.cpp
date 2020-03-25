#include "../inc/ff_gpu.h"

namespace feifei
{
	/************************************************************************/
	/* solution控制															*/
	/************************************************************************/
	SolutionCtrlBase::SolutionCtrlBase(std::string name)
	{
		solutionName = name;
		repeatTimes = 10;
		default_stream = GpuRuntime::GetInstance()->CreateStream(true);
		kernels.clear();
	}
	SolutionCtrlBase::~SolutionCtrlBase()
	{
		for (GpuKernelBase * k : kernels)
		{
			delete k;
		}
		delete default_stream;
	}
	void SolutionCtrlBase::run_comb_kernel()
	{
		// warm up
		launchKernel();

		// verify
		verifyResult();

		LOG("Launch comb kernel for %d times...", repeatTimes);
		elapsedTime = 0.0;
		for (int i = 0; i < repeatTimes; i++)
		{
			launchKernel();
			ffSleepMS(1);
		}

		elapsedTime /= repeatTimes;
		LOG("Elapsed time = " + fmtTime(elapsedTime));

		score.ElapsedTime = elapsedTime;
		//score.TheoryFlops = 64 * 60 * 0.800 * 1000 * 1000 * 1000 * 2;
		score.Flops = score.Calculation / score.ElapsedTime;
		score.Performence = score.Flops / score.TheoryFlops;
		LOG("Calculation = %.2f G", score.Calculation / 1000 / 1000 / 1000);
		LOG("Performance = %.2f TFLOPs", score.Flops / 1000 / 1000 / 1000 / 1000);
		LOG("Theory Performance = %.2f TFLOPs", score.TheoryFlops / 1000 / 1000 / 1000 / 1000);
		LOG("Efficiency = %.2f%%", score.Performence * 100);
	}
	void SolutionCtrlBase::run_mult_kernel()
	{
		for (int i = 0; i < kernels.size(); i++)
		{
			// warm up
			launchSingleKernel(i);
			// verify
			verifyResult();

			LOG("Launch kernel for %d times...", repeatTimes);
			elapsedTime = 0.0;
			for (int t = 0; t < repeatTimes; t++)
			{
				launchSingleKernel(i);
				ffSleepMS(1);
			}
			elapsedTime /= repeatTimes;
			LOG("elapsed time = " + fmtTime(elapsedTime));
		}
	}
	void SolutionCtrlBase::RunSolution()
	{
		PrintSeperator('=');
		INFO("= Solution Name: %s.", solutionName.c_str());
		PrintSeperator('=');

		if (generateKernel() != E_ReturnState::SUCCESS)
			return;

		if (kernels.size() == 0)
			return;

		if (isCombKernels)
			run_comb_kernel();
		else
			run_mult_kernel();
	}
	E_ReturnState SolutionCtrlBase::generateKernel()
	{
		LOG("Generate program and build kernel."); return E_ReturnState::SUCCESS; 
	}
	E_ReturnState SolutionCtrlBase::verifyResult() 
	{
		LOG("Verify Result."); return E_ReturnState::RTN_WARN; 
	}
	E_ReturnState SolutionCtrlBase::launchSingleKernel(unsigned int idx)
	{
		default_stream->Launch(kernels[idx], dispatches[idx].global_size, dispatches[idx].group_size);
		elapsedTime += default_stream->KernelExeTime();

		return E_ReturnState::RTN_ERR;
	}
	E_ReturnState SolutionCtrlBase::launchKernel()
	{
		for (int i = 0; i < kernels.size(); i++)
		{
			default_stream->Launch(kernels[i], dispatches[i].global_size, dispatches[i].group_size);
			elapsedTime += default_stream->KernelExeTime();
		}
		return E_ReturnState::RTN_ERR;
	}
	
	/************************************************************************/
	/* solver 控制															*/
	/************************************************************************/
	SolverCtrlBase::SolverCtrlBase()
	{
		solutions.clear();
	}
	SolverCtrlBase::~SolverCtrlBase()
	{
		for (SolutionCtrlBase * s : solutions)
		{
			delete s;
		}
	}
	void SolverCtrlBase::RunSolver()
	{
		// add solutions to solver
		generateSolver();

		for (SolutionCtrlBase * solution : solutions)
		{
			solution->RunSolution();
		}
	}
	void SolverCtrlBase::generateSolver()
	{ 
		LOG("Generate Solver."); 
	}

	/************************************************************************/
	/* problem 控制															*/
	/************************************************************************/
	ProblemCtrlBase::ProblemCtrlBase(std::string name)
	{
		problemName = name;
		dataMems.clear();
	}
	ProblemCtrlBase::~ProblemCtrlBase()
	{
		for (DataMem<void*> * data_mem : dataMems)
		{
			delete data_mem;
		}

		delete solver;
	}
	void ProblemCtrlBase::RunProblem()
	{
		PrintSeperator('*');
		INFO("* Problem Name: %s.", problemName.c_str());
		PrintSeperator('*');

		initDataMem();
		cpuCompute();
		solver->RunSolver();

		ffSleepSec(1);
	}
	DataMem<cplx_fp32> * ProblemCtrlBase::newCplxData(std::string name, uint32_t dim0, uint32_t dim1, uint32_t dim2)
	{
		DataMem<cplx_fp32> * data_mem = InitCplxData(name, dim0, dim1, dim2);
		DataMem<void*> * t_data = (DataMem<void*>*)data_mem;
		dataMems.push_back(t_data);
		return data_mem;
	};
	void ProblemCtrlBase::initDataMem() { LOG("Initialize DataMem."); }
	void ProblemCtrlBase::cpuCompute() { LOG("Run CPU Calculate."); }


	/************************************************************************/
	/* 添加测试																*/
	/************************************************************************/
	std::vector<ProblemCtrlBase *> g_TestList;

	void addTestExample(ProblemCtrlBase * p)
	{
		g_TestList.push_back(p);
	}
	void RunGpuExample()
	{
		for (ProblemCtrlBase * p : g_TestList)
			p->RunProblem();

		for (ProblemCtrlBase * p : g_TestList)
			delete p;
	}
}

#pragma once

#include "../inc/ff_utils.h"
#include "../inc/ff_gpu.h"
#include "HgemmMfmaKernelWriter.h"

using namespace feifei;

/************************************************************************/
/* solution控制                                                          */
/************************************************************************/
class GemmMfmaAsmSolution : public SolutionCtrlBase
{
public:
	GemmMfmaAsmSolution() : SolutionCtrlBase("asm auto-gen") {}

protected:
	E_ReturnState generateKernel();
	E_ReturnState verifyResult();

private:
	T_GemmMfmaKernelParam kernelParam;
	GemmMfmaKernelWriter * kernelWriter;
};

/************************************************************************/
/* solver 控制															*/
/************************************************************************/
class GemmMfmaSolver : public SolverCtrlBase
{
public:
	GemmMfmaSolver() : SolverCtrlBase() {}

protected:
	void generateSolver();
};

/************************************************************************/
/* 问题控制                                                             */
/************************************************************************/
class GemmMfmaProblem : public ProblemCtrlBase
{
public:
	GemmMfmaProblem() : ProblemCtrlBase("mfma gemm")
	{
		solver = new GemmMfmaSolver();
	}

private:
	void initDataMem();
	void cpuCompute();
};

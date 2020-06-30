.amdgcn_target "amdgcn-amd-amdhsa--gfx908+sram-ecc"
.text
.protected Cijk_Alik_Bljk_BBH_MT64x32x128_SE_K1
.globl Cijk_Alik_Bljk_BBH_MT64x32x128_SE_K1
.p2align 8
.type Cijk_Alik_Bljk_BBH_MT64x32x128_SE_K1,@function
.section .rodata,#alloc
.p2align 6
.amdhsa_kernel Cijk_Alik_Bljk_BBH_MT64x32x128_SE_K1
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_next_free_vgpr 32
  .amdhsa_next_free_sgpr 55
  .amdhsa_group_segment_fixed_size 50688
  .amdhsa_private_segment_fixed_size 0
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_sgpr_workgroup_id_z 1
  .amdhsa_system_vgpr_workitem_id 2
.end_amdhsa_kernel
.text

.amdgpu_metadata
---
amdhsa.version: [ 1, 0 ]
amdhsa.kernels:
  - .name: Cijk_Alik_Bljk_BBH_MT64x32x128_SE_K1
    .symbol: 'Cijk_Alik_Bljk_BBH_MT64x32x128_SE_K1.kd'
    .language: OpenCL C
    .language_version: [ 2, 0 ]
    .group_segment_fixed_size: 50688
    .kernarg_segment_align: 8
    .kernarg_segment_size: 148
    .max_flat_workgroup_size: 1024
    .private_segment_fixed_size: 0
    .sgpr_count: 55
    .sgpr_spill_count: 0
    .vgpr_count: 32
    .vgpr_spill_count: 0
    .wavefront_size: 64
    .args:
      - .name: sizeC
        .size: 8
        .offset: 0
        .value_kind: by_value
        .value_type: u64
      - .name: sizeA
        .size: 8
        .offset: 8
        .value_kind: by_value
        .value_type: u64
      - .name: sizeB
        .size: 8
        .offset: 16
        .value_kind: by_value
        .value_type: u64
      - .name: D
        .size: 8
        .offset: 24
        .value_kind: global_buffer
        .value_type: f32
        .address_space: generic
      - .name: C
        .size: 8
        .offset: 32
        .value_kind: global_buffer
        .value_type: f32
        .address_space: generic
      - .name: A
        .size: 8
        .offset: 40
        .value_kind: global_buffer
        .value_type: f32
        .address_space: generic
      - .name: B
        .size: 8
        .offset: 48
        .value_kind: global_buffer
        .value_type: f32
        .address_space: generic
      - .name: alpha
        .size: 4
        .offset: 56
        .value_kind: by_value
        .value_type: u32
      - .name: beta
        .size: 4
        .offset: 60
        .value_kind: by_value
        .value_type: u32
      - .name: strideD0
        .size: 4
        .offset: 64
        .value_kind: by_value
        .value_type: u32
      - .name: strideD1
        .size: 4
        .offset: 68
        .value_kind: by_value
        .value_type: u32
      - .name: StrideC0
        .size: 4
        .offset: 72
        .value_kind: by_value
        .value_type: u32
      - .name: StrideC1
        .size: 4
        .offset: 76
        .value_kind: by_value
        .value_type: u32
      - .name: StrideA0
        .size: 4
        .offset: 80
        .value_kind: by_value
        .value_type: u32
      - .name: StrideA1
        .size: 4
        .offset: 84
        .value_kind: by_value
        .value_type: u32
      - .name: StrideB0
        .size: 4
        .offset: 88
        .value_kind: by_value
        .value_type: u32
      - .name: StrideB1
        .size: 4
        .offset: 92
        .value_kind: by_value
        .value_type: u32
      - .name: SizesFree0
        .size: 4
        .offset: 96
        .value_kind: by_value
        .value_type: u32
      - .name: SizesFree1
        .size: 4
        .offset: 100
        .value_kind: by_value
        .value_type: u32
      - .name: SizesFree2
        .size: 4
        .offset: 104
        .value_kind: by_value
        .value_type: u32
      - .name: SizesSum0
        .size: 4
        .offset: 108
        .value_kind: by_value
        .value_type: u32
      - .name: OrigStaggerUIter
        .size: 4
        .offset: 112
        .value_kind: by_value
        .value_type: u32
      - .name: NumWorkGroups0
        .size: 4
        .offset: 116
        .value_kind: by_value
        .value_type: u32
      - .name: NumWorkGroups1
        .size: 4
        .offset: 120
        .value_kind: by_value
        .value_type: u32
      - .name: MagicNumberProblemNumGroupTiles0
        .size: 4
        .offset: 124
        .value_kind: by_value
        .value_type: u32
      - .name: GridNumWorkGroups0
        .size: 4
        .offset: 128
        .value_kind: by_value
        .value_type: u32
      - .name: NumFullBlocks
        .size: 4
        .offset: 132
        .value_kind: by_value
        .value_type: u32
      - .name: WgmRemainder1
        .size: 4
        .offset: 136
        .value_kind: by_value
        .value_type: u32
      - .name: MagicNumberWgmRemainder1
        .size: 4
        .offset: 140
        .value_kind: by_value
        .value_type: u32
      - .name: padding
        .size: 4
        .offset: 144
        .value_kind: by_value
        .value_type: u32
...
.end_amdgpu_metadata
Cijk_Alik_Bljk_BBH_MT64x32x128_SE_K1:
START_PROG:
    v_and_b32                                   v[6], 63, v[0]
    v_lshrrev_b32                               v[7], 6, v[0]
    v_readfirstlane_b32                         s[26], v[7]
    s_mul_i32                                   s[5], 32, s[3]
    s_add_u32                                   s[27], s[5], s[2]
    s_mul_i32                                   s[5], 8, s[27]
    s_add_u32                                   s[28], s[5], s[26]
    s_lshr_b32                                  s[29], s[26], 3
    s_and_b32                                   s[5], 7, s[26]
    s_mov_b32                                   s[26], s[5]
    v_mov_b32                                   v[7], s[5]
    v_lshlrev_b32                               v[3], 6, v[7]
    v_add_u32                                   v[0], v[3], v[6]
    s_cmp_eq_u32                                s[29], 0
    s_cbranch_scc1                              MATH_WAVE
    s_load_dwordx2                              s[10:11], s[0:1], 40
    s_load_dwordx2                              s[12:13], s[0:1], 48
    s_load_dwordx4                              s[20:23], s[0:1], 80
    s_mov_b32                                   s[5], 2112
    s_mul_i32                                   s[30], s[26], s[5]
    s_add_i32                                   s[31], s[30], 16896
    s_xor_b32                                   s[32], s[30], s[31]
    s_mov_b32                                   s[29], s[30]
    s_mov_b32                                   s[5], 1056
    s_mul_i32                                   s[34], s[26], s[5]
    s_mov_b32                                   s[5], 33792
    s_add_u32                                   s[34], s[34], s[5]
    s_add_i32                                   s[35], s[34], 8448
    s_xor_b32                                   s[36], s[34], s[35]
    s_mov_b32                                   s[33], s[34]
    s_waitcnt                                   lgkmcnt(0)
    s_mov_b32                                   s[40:40], s[10:10]
    s_mov_b32                                   s[41:41], s[11:11]
    s_mov_b32                                   s[42:42], 0x0+2147483648
    s_mov_b32                                   s[43:43], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[21:21]
    s_lshl_b32                                  s[5], s[5], 1
    s_add_u32                                   s[40:40], s[40:40], s[5]
    s_addc_u32                                  s[41:41], s[41:41], 0
    s_lshl_b32                                  s[37], s[2], 6
    v_lshlrev_b32                               v[8], 3, s[26]
    v_lshrrev_b32                               v[9], 6, v[6]
    v_and_b32                                   v[10], 63, v[6]
    v_add3_u32                                  v[11], s[37], v[8], v[9]                 
    v_mul_lo_u32                                v[3], s[20:20], v[11]
    v_lshlrev_b32                               v[12], 1, v[3]
    v_lshlrev_b32                               v[13], 2, v[10]
    s_lshl_b32                                  s[38], s[20:20], 1
    s_sub_u32                                   s[38], s[38], 264
    v_add_u32                                   v[14], v[12], v[13]
    v_add_u32                                   v[15], s[38], v[14]
    v_add_u32                                   v[16], s[38], v[15]
    v_add_u32                                   v[17], s[38], v[16]
    v_add_u32                                   v[18], s[38], v[17]
    v_add_u32                                   v[19], s[38], v[18]
    v_add_u32                                   v[20], s[38], v[19]
    v_add_u32                                   v[21], s[38], v[20]
    s_mov_b32                                   s[44:44], s[12:12]
    s_mov_b32                                   s[45:45], s[13:13]
    s_mov_b32                                   s[46:46], 0x0+2147483648
    s_mov_b32                                   s[47:47], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[23:23]
    s_lshl_b32                                  s[5], s[5], 1
    s_add_u32                                   s[44:44], s[44:44], s[5]
    s_addc_u32                                  s[45:45], s[45:45], 0
    s_lshl_b32                                  s[37], s[3], 5
    v_lshlrev_b32                               v[8], 2, s[26]
    v_lshrrev_b32                               v[9], 6, v[6]
    v_and_b32                                   v[10], 63, v[6]
    v_add3_u32                                  v[11], s[37], v[8], v[9]                 
    v_mul_lo_u32                                v[3], s[22:22], v[11]
    v_lshlrev_b32                               v[12], 1, v[3]
    v_lshlrev_b32                               v[13], 2, v[10]
    s_lshl_b32                                  s[38], s[22:22], 1
    s_sub_u32                                   s[38], s[38], 264
    v_add_u32                                   v[22], v[12], v[13]
    v_add_u32                                   v[23], s[38], v[22]
    v_add_u32                                   v[24], s[38], v[23]
    v_add_u32                                   v[25], s[38], v[24]
    s_mov_b32                                   s[38], 0
    s_lshr_b32                                  s[39], 480, 7
    s_sub_u32                                   s[48], s[39], 1
FETCH_LOOP:
    s_mov_b32                                   m0, s[29]
    buffer_load_dword                           v[3], v[14], s[40:43], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[15], s[40:43], 0 offen offset:264 lds
    buffer_load_dword                           v[3], v[16], s[40:43], 0 offen offset:528 lds
    buffer_load_dword                           v[3], v[17], s[40:43], 0 offen offset:792 lds
    buffer_load_dword                           v[3], v[18], s[40:43], 0 offen offset:1056 lds
    buffer_load_dword                           v[3], v[19], s[40:43], 0 offen offset:1320 lds
    buffer_load_dword                           v[3], v[20], s[40:43], 0 offen offset:1584 lds
    buffer_load_dword                           v[3], v[21], s[40:43], 0 offen offset:1848 lds
    s_mov_b32                                   m0, s[33]
    buffer_load_dword                           v[3], v[22], s[44:47], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[23], s[44:47], 0 offen offset:264 lds
    buffer_load_dword                           v[3], v[24], s[44:47], 0 offen offset:528 lds
    buffer_load_dword                           v[3], v[25], s[44:47], 0 offen offset:792 lds
    s_xor_b32                                   s[29], s[32], s[29]
    s_xor_b32                                   s[33], s[36], s[33]
    s_xor_b32                                   s[31], s[32], s[29]
    s_xor_b32                                   s[35], s[36], s[33]
    v_add_u32                                   v[14], 256, v[14]
    v_add_u32                                   v[15], 256, v[15]
    v_add_u32                                   v[16], 256, v[16]
    v_add_u32                                   v[17], 256, v[17]
    v_add_u32                                   v[18], 256, v[18]
    v_add_u32                                   v[19], 256, v[19]
    v_add_u32                                   v[20], 256, v[20]
    v_add_u32                                   v[21], 256, v[21]
    v_add_u32                                   v[22], 256, v[22]
    v_add_u32                                   v[23], 256, v[23]
    v_add_u32                                   v[24], 256, v[24]
    v_add_u32                                   v[25], 256, v[25]
    s_waitcnt                                   vmcnt(0)
    s_barrier
    s_sub_u32                                   s[48], s[48], 1
    s_cbranch_scc0                              FETCH_LOOP
END_FETCH_LOOP:
    s_endpgm
MATH_WAVE:
    v_accvgpr_write                             acc[0], 0
    v_accvgpr_write                             acc[1], 0
    v_accvgpr_write                             acc[2], 0
    v_accvgpr_write                             acc[3], 0
    s_load_dwordx2                              s[8:9], s[0:1], 32
    s_load_dwordx2                              s[6:7], s[0:1], 24
    s_load_dwordx2                              s[14:15], s[0:1], 56
    s_load_dwordx4                              s[16:19], s[0:1], 64
    v_and_b32                                   v[8], 3, s[26]
    v_lshlrev_b32                               v[9], 4, v[8]
    v_and_b32                                   v[10], v[6], 15
    v_lshrrev_b32                               v[11], 4, v[6]
    v_add_u32                                   v[12], v[9], v[10]
    v_lshlrev_b32                               v[3], 7, v[12]
    v_lshlrev_b32                               v[13], 1, v[3]
    v_lshlrev_b32                               v[14], 2, v[11]
    v_lshrrev_b32                               v[3], 0, v[12]
    v_mul_lo_u32                                v[15], 8, v[3]
    v_add_u32                                   v[17], v[13], v[14]
    v_add_u32                                   v[17], v[17], v[15]
    v_add_u32                                   v[18], 16896, v[17]
    v_xor_b32                                   v[19], v[18], v[17]
    v_mov_b32                                   v[16], v[17]
    v_lshrrev_b32                               v[8], 2, s[26]
    v_lshlrev_b32                               v[9], 4, v[8]
    v_and_b32                                   v[10], v[6], 15
    v_lshrrev_b32                               v[11], 4, v[6]
    v_add_u32                                   v[12], v[9], v[10]
    v_lshlrev_b32                               v[3], 7, v[12]
    v_lshlrev_b32                               v[13], 1, v[3]
    v_lshlrev_b32                               v[14], 2, v[11]
    v_lshrrev_b32                               v[3], 0, v[12]
    v_mul_lo_u32                                v[15], 8, v[3]
    v_add_u32                                   v[21], v[13], v[14]
    v_mov_b32                                   v[3], 0x0+33792
    v_add_u32                                   v[21], v[21], v[3]
    v_add_u32                                   v[21], v[21], v[15]
    v_add_u32                                   v[22], 8448, v[21]
    v_xor_b32                                   v[23], v[22], v[21]
    v_mov_b32                                   v[20], v[21]
    s_waitcnt                                   lgkmcnt(0)
    s_mov_b32                                   s[32:32], s[8:8]
    s_mov_b32                                   s[33:33], s[9:9]
    s_mov_b32                                   s[34:34], 0x0+2147483648
    s_mov_b32                                   s[35:35], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[19:19]
    s_lshl_b32                                  s[5], s[5], 2
    s_add_u32                                   s[32:32], s[32:32], s[5]
    s_addc_u32                                  s[33:33], s[33:33], 0
    s_lshl_b32                                  s[30], s[2], 6
    s_lshl_b32                                  s[29], s[3], 5
    s_lshr_b32                                  s[5], s[26], 2
    v_lshlrev_b32                               v[8], 4, s[5]
    s_and_b32                                   s[5], s[26], 3
    v_lshlrev_b32                               v[9], 4, s[5]
    v_and_b32                                   v[10], 15, v[6]
    v_lshrrev_b32                               v[11], 4, v[6]
    v_lshlrev_b32                               v[12], 2, v[11]
    v_add3_u32                                  v[13], s[29], v[8], v[10]                
    v_add3_u32                                  v[14], s[30], v[9], v[12]                
    v_mul_lo_u32                                v[3], s[18:18], v[13]
    v_lshlrev_b32                               v[15], 2, v[3]
    v_lshlrev_b32                               v[24], 2, v[14]
    v_add_u32                                   v[25], v[15], v[24]
    v_lshlrev_b32                               v[3], 4, s[18:18]
    v_lshlrev_b32                               v[26], 2, v[3]
    v_mov_b32                                   v[27], v[25]
    s_mov_b32                                   s[36:36], s[6:6]
    s_mov_b32                                   s[37:37], s[7:7]
    s_mov_b32                                   s[38:38], 0x0+2147483648
    s_mov_b32                                   s[39:39], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[17:17]
    s_lshl_b32                                  s[5], s[5], 1
    s_add_u32                                   s[36:36], s[36:36], s[5]
    s_addc_u32                                  s[37:37], s[37:37], 0
    s_lshl_b32                                  s[30], s[2], 5
    s_lshl_b32                                  s[29], s[3], 5
    s_lshr_b32                                  s[5], s[26], 2
    v_lshlrev_b32                               v[8], 4, s[5]
    s_and_b32                                   s[5], s[26], 3
    v_lshlrev_b32                               v[9], 3, s[5]
    v_and_b32                                   v[10], 15, v[6]
    v_lshrrev_b32                               v[11], 4, v[6]
    v_lshlrev_b32                               v[12], 1, v[11]
    v_add3_u32                                  v[13], s[29], v[8], v[10]                
    v_add3_u32                                  v[14], s[30], v[9], v[12]                
    v_mul_lo_u32                                v[3], s[16:16], v[13]
    v_lshlrev_b32                               v[15], 1, v[3]
    v_lshlrev_b32                               v[24], 2, v[14]
    v_lshlrev_b32                               v[3], 4, s[16:16]
    v_lshlrev_b32                               v[25], 1, v[3]
    v_add_u32                                   v[26], v[15], v[24]
    buffer_load_dwordx4                         v[28:31], v[27], s[32:35], 0 offen offset:0
    s_waitcnt                                   vmcnt(0)
    v_mul_f32                                   v[28], s[15:15], v[28]
    v_mul_f32                                   v[29], s[15:15], v[29]
    v_mul_f32                                   v[30], s[15:15], v[30]
    v_mul_f32                                   v[31], s[15:15], v[31]
    s_mov_b32                                   s[30], 0
    s_setprio                                   1
    s_barrier
    ds_read_b32                                 v[8], v[16]
    ds_read_b32                                 v[9], v[20]
    s_lshr_b32                                  s[5], 480, 7
    s_cmp_le_u32                                s[5], 1
    s_cbranch_scc1                              END_MATH_LOOP
    s_sub_u32                                   s[5], s[5], 1
    s_sub_u32                                   s[31], s[5], 1
MATH_LOOP:
    ds_read_b32                                 v[10], v[16]                             offset:16
    ds_read_b32                                 v[11], v[20]                             offset:16
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:32
    ds_read_b32                                 v[9], v[20]                              offset:32
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:48
    ds_read_b32                                 v[11], v[20]                             offset:48
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:64
    ds_read_b32                                 v[9], v[20]                              offset:64
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:80
    ds_read_b32                                 v[11], v[20]                             offset:80
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:96
    ds_read_b32                                 v[9], v[20]                              offset:96
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:112
    ds_read_b32                                 v[11], v[20]                             offset:112
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:128
    ds_read_b32                                 v[9], v[20]                              offset:128
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:144
    ds_read_b32                                 v[11], v[20]                             offset:144
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:160
    ds_read_b32                                 v[9], v[20]                              offset:160
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:176
    ds_read_b32                                 v[11], v[20]                             offset:176
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:192
    ds_read_b32                                 v[9], v[20]                              offset:192
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:208
    ds_read_b32                                 v[11], v[20]                             offset:208
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:224
    ds_read_b32                                 v[9], v[20]                              offset:224
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:240
    ds_read_b32                                 v[11], v[20]                             offset:240
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    v_xor_b32                                   v[16], v[19], v[16]
    v_xor_b32                                   v[20], v[23], v[20]
    v_xor_b32                                   v[18], v[19], v[16]
    v_xor_b32                                   v[22], v[23], v[20]
    s_waitcnt                                   lgkmcnt(0)
    s_barrier
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[8], v[16]
    ds_read_b32                                 v[9], v[20]
    s_sub_u32                                   s[31], s[31], 1
    s_cbranch_scc0                              MATH_LOOP
END_MATH_LOOP:
    ds_read_b32                                 v[10], v[16]                             offset:16
    ds_read_b32                                 v[11], v[20]                             offset:16
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:32
    ds_read_b32                                 v[9], v[20]                              offset:32
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:48
    ds_read_b32                                 v[11], v[20]                             offset:48
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:64
    ds_read_b32                                 v[9], v[20]                              offset:64
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:80
    ds_read_b32                                 v[11], v[20]                             offset:80
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:96
    ds_read_b32                                 v[9], v[20]                              offset:96
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:112
    ds_read_b32                                 v[11], v[20]                             offset:112
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:128
    ds_read_b32                                 v[9], v[20]                              offset:128
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:144
    ds_read_b32                                 v[11], v[20]                             offset:144
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:160
    ds_read_b32                                 v[9], v[20]                              offset:160
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:176
    ds_read_b32                                 v[11], v[20]                             offset:176
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:192
    ds_read_b32                                 v[9], v[20]                              offset:192
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:208
    ds_read_b32                                 v[11], v[20]                             offset:208
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    ds_read_b32                                 v[8], v[16]                              offset:224
    ds_read_b32                                 v[9], v[20]                              offset:224
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    ds_read_b32                                 v[10], v[16]                             offset:240
    ds_read_b32                                 v[11], v[20]                             offset:240
    s_waitcnt                                   lgkmcnt(2)
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[8:8], v[9:9], acc[0:3]       
    s_waitcnt                                   lgkmcnt(0)
    v_mov_b32                                   v[24], 0x0+4294901760
    v_mfma_f32_16x16x8bf16                      acc[0:3], v[10:10], v[11:11], acc[0:3]   
    s_nop                                       32
    v_accvgpr_read                              v[12], acc[0:0]
    s_nop                                       4
    v_fma_f32                                   v[12], s[14:14], v[12], v[28]            
    v_accvgpr_read                              v[13], acc[1:1]
    s_nop                                       4
    v_fma_f32                                   v[13], s[14:14], v[13], v[29]            
    v_lshrrev_b32                               v[12], 16, v[12]
    v_and_or_b32                                v[12], v[13], v[24], v[12]               
    v_accvgpr_read                              v[14], acc[2:2]
    s_nop                                       4
    v_fma_f32                                   v[14], s[14:14], v[14], v[30]            
    v_accvgpr_read                              v[15], acc[3:3]
    s_nop                                       4
    v_fma_f32                                   v[15], s[14:14], v[15], v[31]            
    v_lshrrev_b32                               v[14], 16, v[14]
    v_and_or_b32                                v[13], v[15], v[24], v[14]               
    buffer_store_dwordx2                        v[12:13], v[26], s[36:39], 0 offen offset:0
END_PROG:
    s_endpgm


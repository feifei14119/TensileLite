.amdgcn_target "amdgcn-amd-amdhsa--gfx908+sram-ecc"
.text
.protected Cijk_Alik_Bljk_HBH_MT32x64x32_SE_K1
.globl Cijk_Alik_Bljk_HBH_MT32x64x32_SE_K1
.p2align 8
.type Cijk_Alik_Bljk_HBH_MT32x64x32_SE_K1,@function
.section .rodata,#alloc
.p2align 6
.amdhsa_kernel Cijk_Alik_Bljk_HBH_MT32x64x32_SE_K1
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_next_free_vgpr 45
  .amdhsa_next_free_sgpr 55
  .amdhsa_group_segment_fixed_size 19008
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
  - .name: Cijk_Alik_Bljk_HBH_MT32x64x32_SE_K1
    .symbol: 'Cijk_Alik_Bljk_HBH_MT32x64x32_SE_K1.kd'
    .language: OpenCL C
    .language_version: [ 2, 0 ]
    .group_segment_fixed_size: 19008
    .kernarg_segment_align: 8
    .kernarg_segment_size: 148
    .max_flat_workgroup_size: 512
    .private_segment_fixed_size: 0
    .sgpr_count: 55
    .sgpr_spill_count: 0
    .vgpr_count: 45
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
Cijk_Alik_Bljk_HBH_MT32x64x32_SE_K1:
START_PROG:
    v_and_b32                                   v[6], 63, v[0]
    v_lshrrev_b32                               v[7], 6, v[0]
    v_readfirstlane_b32                         s[26], v[7]
    s_mul_i32                                   s[5], 15, s[3]
    s_add_u32                                   s[27], s[5], s[2]
    s_mul_i32                                   s[5], 4, s[27]
    s_add_u32                                   s[28], s[5], s[26]
    s_lshr_b32                                  s[29], s[26], 2
    s_and_b32                                   s[5], 3, s[26]
    s_mov_b32                                   s[26], s[5]
    v_mov_b32                                   v[7], s[5]
    v_lshlrev_b32                               v[3], 6, v[7]
    v_add_u32                                   v[0], v[3], v[6]
    s_cmp_eq_u32                                s[29], 0
    s_cbranch_scc1                              MATH_WAVE
    s_load_dwordx2                              s[10:11], s[0:1], 40
    s_load_dwordx2                              s[12:13], s[0:1], 48
    s_load_dwordx4                              s[20:23], s[0:1], 80
    s_mov_b32                                   s[5], 528
    s_mul_i32                                   s[30], s[26], s[5]
    s_mov_b32                                   s[32], 2112
    s_mov_b32                                   s[29], s[30]
    s_mov_b32                                   s[5], 1056
    s_mul_i32                                   s[34], s[26], s[5]
    s_mov_b32                                   s[5], 6336
    s_add_u32                                   s[34], s[34], s[5]
    s_mov_b32                                   s[36], 4224
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
    s_lshl_b32                                  s[37], s[2], 5
    v_lshlrev_b32                               v[8], 3, s[26]
    v_lshrrev_b32                               v[9], 4, v[6]
    v_and_b32                                   v[10], 15, v[6]
    v_add3_u32                                  v[11], s[37], v[8], v[9]                 
    v_mul_lo_u32                                v[3], s[20:20], v[11]
    v_lshlrev_b32                               v[12], 1, v[3]
    v_lshlrev_b32                               v[13], 2, v[10]
    s_lshl_b32                                  s[38], s[20:20], 3
    s_sub_u32                                   s[38], s[38], 264
    v_add_u32                                   v[14], v[12], v[13]
    v_add_u32                                   v[15], s[38], v[14]
    s_mov_b32                                   s[44:44], s[12:12]
    s_mov_b32                                   s[45:45], s[13:13]
    s_mov_b32                                   s[46:46], 0x0+2147483648
    s_mov_b32                                   s[47:47], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[23:23]
    s_lshl_b32                                  s[5], s[5], 1
    s_add_u32                                   s[44:44], s[44:44], s[5]
    s_addc_u32                                  s[45:45], s[45:45], 0
    s_lshl_b32                                  s[37], s[3], 6
    v_lshlrev_b32                               v[8], 4, s[26]
    v_lshrrev_b32                               v[9], 4, v[6]
    v_and_b32                                   v[10], 15, v[6]
    v_add3_u32                                  v[11], s[37], v[8], v[9]                 
    v_mul_lo_u32                                v[3], s[22:22], v[11]
    v_lshlrev_b32                               v[12], 1, v[3]
    v_lshlrev_b32                               v[13], 2, v[10]
    s_lshl_b32                                  s[38], s[22:22], 3
    s_sub_u32                                   s[38], s[38], 264
    v_add_u32                                   v[16], v[12], v[13]
    v_add_u32                                   v[17], s[38], v[16]
    v_add_u32                                   v[18], s[38], v[17]
    v_add_u32                                   v[19], s[38], v[18]
    s_mov_b32                                   s[38], 0
    s_lshr_b32                                  s[39], 1024, 5
    s_cmp_le_u32                                s[39], 1
    s_cbranch_scc1                              END_FETCH_LOOP
    s_sub_u32                                   s[39], s[39], 1
    s_mov_b32                                   m0, s[29]
    buffer_load_dword                           v[3], v[14], s[40:43], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[15], s[40:43], 0 offen offset:264 lds
    s_mov_b32                                   m0, s[33]
    buffer_load_dword                           v[3], v[16], s[44:47], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[17], s[44:47], 0 offen offset:264 lds
    buffer_load_dword                           v[3], v[18], s[44:47], 0 offen offset:528 lds
    buffer_load_dword                           v[3], v[19], s[44:47], 0 offen offset:792 lds
    s_add_u32                                   s[38], s[38], 1
    s_cmp_eq_i32                                s[38], 3
    s_cselect_b32                               s[37], 0, s[38]
    s_mov_b32                                   s[38], s[37]
    s_mul_i32                                   s[31], s[32], s[37]
    s_add_u32                                   s[29], s[30], s[31]
    s_mul_i32                                   s[35], s[36], s[37]
    s_add_u32                                   s[33], s[34], s[35]
    v_add_u32                                   v[14], 64, v[14]
    v_add_u32                                   v[15], 64, v[15]
    v_add_u32                                   v[16], 64, v[16]
    v_add_u32                                   v[17], 64, v[17]
    v_add_u32                                   v[18], 64, v[18]
    v_add_u32                                   v[19], 64, v[19]
    s_sub_u32                                   s[48], s[39], 1
FETCH_LOOP:
    s_mov_b32                                   m0, s[29]
    buffer_load_dword                           v[3], v[14], s[40:43], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[15], s[40:43], 0 offen offset:264 lds
    s_mov_b32                                   m0, s[33]
    buffer_load_dword                           v[3], v[16], s[44:47], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[17], s[44:47], 0 offen offset:264 lds
    buffer_load_dword                           v[3], v[18], s[44:47], 0 offen offset:528 lds
    buffer_load_dword                           v[3], v[19], s[44:47], 0 offen offset:792 lds
    s_add_u32                                   s[38], s[38], 1
    s_cmp_eq_i32                                s[38], 3
    s_cselect_b32                               s[37], 0, s[38]
    s_mov_b32                                   s[38], s[37]
    s_mul_i32                                   s[31], s[32], s[37]
    s_add_u32                                   s[29], s[30], s[31]
    s_mul_i32                                   s[35], s[36], s[37]
    s_add_u32                                   s[33], s[34], s[35]
    v_add_u32                                   v[14], 64, v[14]
    v_add_u32                                   v[15], 64, v[15]
    v_add_u32                                   v[16], 64, v[16]
    v_add_u32                                   v[17], 64, v[17]
    v_add_u32                                   v[18], 64, v[18]
    v_add_u32                                   v[19], 64, v[19]
    s_waitcnt                                   vmcnt(6)
    s_barrier
    s_sub_u32                                   s[48], s[48], 1
    s_cbranch_scc0                              FETCH_LOOP
END_FETCH_LOOP:
    s_waitcnt                                   vmcnt(0)
    s_barrier
    s_endpgm
MATH_WAVE:
    v_accvgpr_write                             acc[0], 0
    v_accvgpr_write                             acc[1], 0
    v_accvgpr_write                             acc[2], 0
    v_accvgpr_write                             acc[3], 0
    v_accvgpr_write                             acc[4], 0
    v_accvgpr_write                             acc[5], 0
    v_accvgpr_write                             acc[6], 0
    v_accvgpr_write                             acc[7], 0
    s_load_dwordx2                              s[8:9], s[0:1], 32
    s_load_dwordx2                              s[6:7], s[0:1], 24
    s_load_dwordx2                              s[14:15], s[0:1], 56
    s_load_dwordx4                              s[16:19], s[0:1], 64
    v_and_b32                                   v[8], 0, s[26]
    v_lshlrev_b32                               v[9], 5, v[8]
    v_and_b32                                   v[10], v[6], 15
    v_lshrrev_b32                               v[11], 4, v[6]
    v_add_u32                                   v[12], v[9], v[10]
    v_lshlrev_b32                               v[3], 5, v[12]
    v_lshlrev_b32                               v[13], 1, v[3]
    v_lshlrev_b32                               v[14], 3, v[11]
    v_lshrrev_b32                               v[3], 2, v[12]
    v_mul_lo_u32                                v[15], 8, v[3]
    v_add_u32                                   v[17], v[13], v[14]
    v_add_u32                                   v[17], v[17], v[15]
    v_mov_b32                                   v[19], 2112
    v_mov_b32                                   v[16], v[17]
    v_lshrrev_b32                               v[8], 0, s[26]
    v_lshlrev_b32                               v[9], 4, v[8]
    v_and_b32                                   v[10], v[6], 15
    v_lshrrev_b32                               v[11], 4, v[6]
    v_add_u32                                   v[12], v[9], v[10]
    v_lshlrev_b32                               v[3], 5, v[12]
    v_lshlrev_b32                               v[13], 1, v[3]
    v_lshlrev_b32                               v[14], 3, v[11]
    v_lshrrev_b32                               v[3], 2, v[12]
    v_mul_lo_u32                                v[15], 8, v[3]
    v_add_u32                                   v[21], v[13], v[14]
    v_mov_b32                                   v[3], 0x0+6336
    v_add_u32                                   v[21], v[21], v[3]
    v_add_u32                                   v[21], v[21], v[15]
    v_mov_b32                                   v[23], 4224
    v_mov_b32                                   v[20], v[21]
    s_waitcnt                                   lgkmcnt(0)
    s_mov_b32                                   s[32:32], s[8:8]
    s_mov_b32                                   s[33:33], s[9:9]
    s_mov_b32                                   s[34:34], 0x0+2147483648
    s_mov_b32                                   s[35:35], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[19:19]
    s_lshl_b32                                  s[5], s[5], 1
    s_add_u32                                   s[32:32], s[32:32], s[5]
    s_addc_u32                                  s[33:33], s[33:33], 0
    s_lshl_b32                                  s[30], s[2], 4
    s_lshl_b32                                  s[29], s[3], 6
    s_lshr_b32                                  s[5], s[26], 0
    v_lshlrev_b32                               v[8], 4, s[5]
    s_and_b32                                   s[5], s[26], 0
    v_lshlrev_b32                               v[9], 4, s[5]
    v_and_b32                                   v[10], 15, v[6]
    v_lshrrev_b32                               v[11], 4, v[6]
    v_lshlrev_b32                               v[12], 1, v[11]
    v_add3_u32                                  v[13], s[29], v[8], v[10]                
    v_add3_u32                                  v[14], s[30], v[9], v[12]                
    v_mul_lo_u32                                v[3], s[18:18], v[13]
    v_lshlrev_b32                               v[15], 1, v[3]
    v_lshlrev_b32                               v[24], 2, v[14]
    v_add_u32                                   v[25], v[15], v[24]
    v_lshlrev_b32                               v[3], 4, s[18:18]
    v_lshlrev_b32                               v[26], 1, v[3]
    v_mov_b32                                   v[27], v[25]
    s_mov_b32                                   s[36:36], s[6:6]
    s_mov_b32                                   s[37:37], s[7:7]
    s_mov_b32                                   s[38:38], 0x0+2147483648
    s_mov_b32                                   s[39:39], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[17:17]
    s_lshl_b32                                  s[5], s[5], 1
    s_add_u32                                   s[36:36], s[36:36], s[5]
    s_addc_u32                                  s[37:37], s[37:37], 0
    s_lshl_b32                                  s[30], s[2], 4
    s_lshl_b32                                  s[29], s[3], 6
    s_lshr_b32                                  s[5], s[26], 0
    v_lshlrev_b32                               v[8], 4, s[5]
    s_and_b32                                   s[5], s[26], 0
    v_lshlrev_b32                               v[9], 4, s[5]
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
    buffer_load_dwordx2                         v[28:29], v[27], s[32:35], 0 offen offset:0
    buffer_load_dwordx2                         v[32:33], v[27], s[32:35], 0 offen offset:32
    v_cvt_f32_f16                               v[3], s[14:14]
    v_cvt_f32_f16                               v[4], s[15:15]
    v_readfirstlane_b32                         s[14:14], v[3]
    v_readfirstlane_b32                         s[15:15], v[4]
    s_waitcnt                                   vmcnt(0)
    v_cvt_f32_f16                               v[3], v[29]
    v_lshrrev_b32                               v[29], 16, v[29]
    v_cvt_f32_f16                               v[31], v[29]
    v_mul_f32                                   v[30], s[15:15], v[3]
    v_mul_f32                                   v[31], s[15:15], v[31]
    v_cvt_f32_f16                               v[3], v[28]
    v_lshrrev_b32                               v[28], 16, v[28]
    v_cvt_f32_f16                               v[29], v[28]
    v_mul_f32                                   v[28], s[15:15], v[3]
    v_mul_f32                                   v[29], s[15:15], v[29]
    s_waitcnt                                   vmcnt(0)
    v_cvt_f32_f16                               v[3], v[33]
    v_lshrrev_b32                               v[33], 16, v[33]
    v_cvt_f32_f16                               v[35], v[33]
    v_mul_f32                                   v[34], s[15:15], v[3]
    v_mul_f32                                   v[35], s[15:15], v[35]
    v_cvt_f32_f16                               v[3], v[32]
    v_lshrrev_b32                               v[32], 16, v[32]
    v_cvt_f32_f16                               v[33], v[32]
    v_mul_f32                                   v[32], s[15:15], v[3]
    v_mul_f32                                   v[33], s[15:15], v[33]
    s_mov_b32                                   s[30], 0
    s_setprio                                   1
    v_mov_b32                                   v[25], 0
    s_barrier
    ds_read_b64                                 v[8:9], v[16]
    ds_read_b64                                 v[10:11], v[16]                          offset:1056
    ds_read_b64                                 v[12:13], v[20]
    s_lshr_b32                                  s[5], 1024, 5
    s_cmp_le_u32                                s[5], 1
    s_cbranch_scc1                              END_MATH_LOOP
    s_sub_u32                                   s[5], s[5], 1
    s_sub_u32                                   s[31], s[5], 1
MATH_LOOP:
    ds_read_b64                                 v[36:37], v[16]                          offset:32
    ds_read_b64                                 v[38:39], v[16]                          offset:1088
    ds_read_b64                                 v[14:15], v[20]                          offset:32
    s_waitcnt                                   lgkmcnt(3)
    v_mfma_f32_16x16x16f16                      acc[0:3], v[8:9], v[12:13], acc[0:3]     
    v_add_u32                                   v[25], v[25], 1
    v_cmp_eq_u32                                vcc, v[25], 3
    v_cndmask_b32                               v[24], v[25], 0, vcc                     
    v_mov_b32                                   v[25], v[24]
    v_mad_u32_u24                               v[16], v[19], v[24], v[17]               
    v_mad_u32_u24                               v[20], v[23], v[24], v[21]               
    v_mfma_f32_16x16x16f16                      acc[4:7], v[10:11], v[12:13], acc[4:7]   
    s_waitcnt                                   lgkmcnt(0)
    s_barrier
    v_mfma_f32_16x16x16f16                      acc[0:3], v[36:37], v[14:15], acc[0:3]   
    ds_read_b64                                 v[8:9], v[16]
    ds_read_b64                                 v[10:11], v[16]                          offset:1056
    ds_read_b64                                 v[12:13], v[20]
    v_mfma_f32_16x16x16f16                      acc[4:7], v[38:39], v[14:15], acc[4:7]   
    s_sub_u32                                   s[31], s[31], 1
    s_cbranch_scc0                              MATH_LOOP
END_MATH_LOOP:
    ds_read_b64                                 v[36:37], v[16]                          offset:32
    ds_read_b64                                 v[38:39], v[16]                          offset:1088
    ds_read_b64                                 v[14:15], v[20]                          offset:32
    s_waitcnt                                   lgkmcnt(3)
    v_mfma_f32_16x16x16f16                      acc[0:3], v[8:9], v[12:13], acc[0:3]     
    v_mfma_f32_16x16x16f16                      acc[4:7], v[10:11], v[12:13], acc[4:7]   
    s_waitcnt                                   lgkmcnt(0)
    v_mov_b32                                   v[44], 0x0+4294901760
    v_mfma_f32_16x16x16f16                      acc[0:3], v[36:37], v[14:15], acc[0:3]   
    v_mfma_f32_16x16x16f16                      acc[4:7], v[38:39], v[14:15], acc[4:7]   
    v_accvgpr_read                              v[40], acc[0:0]
    v_fma_f32                                   v[40], s[14:14], v[40], v[28]            
    v_cvt_f16_f32                               v[40], v[40]
    v_accvgpr_read                              v[41], acc[1:1]
    v_fma_f32                                   v[41], s[14:14], v[41], v[29]            
    v_cvt_f16_f32                               v[41], v[41]
    v_pack_b32_f16                              v[40], v[40], v[41]
    v_accvgpr_read                              v[42], acc[2:2]
    v_fma_f32                                   v[42], s[14:14], v[42], v[30]            
    v_cvt_f16_f32                               v[42], v[42]
    v_accvgpr_read                              v[43], acc[3:3]
    v_fma_f32                                   v[43], s[14:14], v[43], v[31]            
    v_cvt_f16_f32                               v[43], v[43]
    v_pack_b32_f16                              v[41], v[42], v[43]
    buffer_store_dwordx2                        v[40:41], v[26], s[36:39], 0 offen offset:0
    v_accvgpr_read                              v[40], acc[4:4]
    v_fma_f32                                   v[40], s[14:14], v[40], v[32]            
    v_cvt_f16_f32                               v[40], v[40]
    v_accvgpr_read                              v[41], acc[5:5]
    v_fma_f32                                   v[41], s[14:14], v[41], v[33]            
    v_cvt_f16_f32                               v[41], v[41]
    v_pack_b32_f16                              v[40], v[40], v[41]
    v_accvgpr_read                              v[42], acc[6:6]
    v_fma_f32                                   v[42], s[14:14], v[42], v[34]            
    v_cvt_f16_f32                               v[42], v[42]
    v_accvgpr_read                              v[43], acc[7:7]
    v_fma_f32                                   v[43], s[14:14], v[43], v[35]            
    v_cvt_f16_f32                               v[43], v[43]
    v_pack_b32_f16                              v[41], v[42], v[43]
    buffer_store_dwordx2                        v[40:41], v[26], s[36:39], 0 offen offset:32
END_PROG:
    s_endpgm


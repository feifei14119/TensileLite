.amdgcn_target "amdgcn-amd-amdhsa--gfx908+sram-ecc"
.text
.protected Cijk_Alik_Bljk_SB_MT32x128x32_SE_K1
.globl Cijk_Alik_Bljk_SB_MT32x128x32_SE_K1
.p2align 8
.type Cijk_Alik_Bljk_SB_MT32x128x32_SE_K1,@function
.section .rodata,#alloc
.p2align 6
.amdhsa_kernel Cijk_Alik_Bljk_SB_MT32x128x32_SE_K1
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_next_free_vgpr 52
  .amdhsa_next_free_sgpr 55
  .amdhsa_group_segment_fixed_size 63360
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
  - .name: Cijk_Alik_Bljk_SB_MT32x128x32_SE_K1
    .symbol: 'Cijk_Alik_Bljk_SB_MT32x128x32_SE_K1.kd'
    .language: OpenCL C
    .language_version: [ 2, 0 ]
    .group_segment_fixed_size: 63360
    .kernarg_segment_align: 8
    .kernarg_segment_size: 148
    .max_flat_workgroup_size: 512
    .private_segment_fixed_size: 0
    .sgpr_count: 55
    .sgpr_spill_count: 0
    .vgpr_count: 52
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
Cijk_Alik_Bljk_SB_MT32x128x32_SE_K1:
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
    s_mov_b32                                   s[5], 1056
    s_mul_i32                                   s[30], s[26], s[5]
    s_mov_b32                                   s[32], 4224
    s_mov_b32                                   s[29], s[30]
    s_mov_b32                                   s[5], 4224
    s_mul_i32                                   s[34], s[26], s[5]
    s_mov_b32                                   s[5], 12672
    s_add_u32                                   s[34], s[34], s[5]
    s_mov_b32                                   s[36], 16896
    s_mov_b32                                   s[33], s[34]
    s_waitcnt                                   lgkmcnt(0)
    s_mov_b32                                   s[40:40], s[10:10]
    s_mov_b32                                   s[41:41], s[11:11]
    s_mov_b32                                   s[42:42], 0x0+2147483648
    s_mov_b32                                   s[43:43], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[21:21]
    s_lshl_b32                                  s[5], s[5], 2
    s_add_u32                                   s[40:40], s[40:40], s[5]
    s_addc_u32                                  s[41:41], s[41:41], 0
    s_lshl_b32                                  s[37], s[2], 5
    v_lshlrev_b32                               v[8], 3, s[26]
    v_lshrrev_b32                               v[9], 5, v[6]
    v_and_b32                                   v[10], 31, v[6]
    v_add3_u32                                  v[11], s[37], v[8], v[9]                 
    v_mul_lo_u32                                v[3], s[20:20], v[11]
    v_lshlrev_b32                               v[12], 2, v[3]
    v_lshlrev_b32                               v[13], 2, v[10]
    s_lshl_b32                                  s[38], s[20:20], 3
    s_sub_u32                                   s[38], s[38], 264
    v_add_u32                                   v[14], v[12], v[13]
    v_add_u32                                   v[15], s[38], v[14]
    v_add_u32                                   v[16], s[38], v[15]
    v_add_u32                                   v[17], s[38], v[16]
    s_mov_b32                                   s[44:44], s[12:12]
    s_mov_b32                                   s[45:45], s[13:13]
    s_mov_b32                                   s[46:46], 0x0+2147483648
    s_mov_b32                                   s[47:47], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[23:23]
    s_lshl_b32                                  s[5], s[5], 2
    s_add_u32                                   s[44:44], s[44:44], s[5]
    s_addc_u32                                  s[45:45], s[45:45], 0
    s_lshl_b32                                  s[37], s[3], 7
    v_lshlrev_b32                               v[8], 5, s[26]
    v_lshrrev_b32                               v[9], 5, v[6]
    v_and_b32                                   v[10], 31, v[6]
    v_add3_u32                                  v[11], s[37], v[8], v[9]                 
    v_mul_lo_u32                                v[3], s[22:22], v[11]
    v_lshlrev_b32                               v[12], 2, v[3]
    v_lshlrev_b32                               v[13], 2, v[10]
    s_lshl_b32                                  s[38], s[22:22], 3
    s_sub_u32                                   s[38], s[38], 264
    v_add_u32                                   v[18], v[12], v[13]
    v_add_u32                                   v[19], s[38], v[18]
    v_add_u32                                   v[20], s[38], v[19]
    v_add_u32                                   v[21], s[38], v[20]
    v_add_u32                                   v[22], s[38], v[21]
    v_add_u32                                   v[23], s[38], v[22]
    v_add_u32                                   v[24], s[38], v[23]
    v_add_u32                                   v[25], s[38], v[24]
    v_add_u32                                   v[26], s[38], v[25]
    v_add_u32                                   v[27], s[38], v[26]
    v_add_u32                                   v[28], s[38], v[27]
    v_add_u32                                   v[29], s[38], v[28]
    v_add_u32                                   v[30], s[38], v[29]
    v_add_u32                                   v[31], s[38], v[30]
    v_add_u32                                   v[32], s[38], v[31]
    v_add_u32                                   v[33], s[38], v[32]
    s_mov_b32                                   s[38], 0
    s_lshr_b32                                  s[39], 1024, 5
    s_cmp_le_u32                                s[39], 1
    s_cbranch_scc1                              END_FETCH_LOOP
    s_sub_u32                                   s[39], s[39], 1
    s_mov_b32                                   m0, s[29]
    buffer_load_dword                           v[3], v[14], s[40:43], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[15], s[40:43], 0 offen offset:264 lds
    buffer_load_dword                           v[3], v[16], s[40:43], 0 offen offset:528 lds
    buffer_load_dword                           v[3], v[17], s[40:43], 0 offen offset:792 lds
    s_mov_b32                                   m0, s[33]
    buffer_load_dword                           v[3], v[18], s[44:47], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[19], s[44:47], 0 offen offset:264 lds
    buffer_load_dword                           v[3], v[20], s[44:47], 0 offen offset:528 lds
    buffer_load_dword                           v[3], v[21], s[44:47], 0 offen offset:792 lds
    buffer_load_dword                           v[3], v[22], s[44:47], 0 offen offset:1056 lds
    buffer_load_dword                           v[3], v[23], s[44:47], 0 offen offset:1320 lds
    buffer_load_dword                           v[3], v[24], s[44:47], 0 offen offset:1584 lds
    buffer_load_dword                           v[3], v[25], s[44:47], 0 offen offset:1848 lds
    buffer_load_dword                           v[3], v[26], s[44:47], 0 offen offset:2112 lds
    buffer_load_dword                           v[3], v[27], s[44:47], 0 offen offset:2376 lds
    buffer_load_dword                           v[3], v[28], s[44:47], 0 offen offset:2640 lds
    buffer_load_dword                           v[3], v[29], s[44:47], 0 offen offset:2904 lds
    buffer_load_dword                           v[3], v[30], s[44:47], 0 offen offset:3168 lds
    buffer_load_dword                           v[3], v[31], s[44:47], 0 offen offset:3432 lds
    buffer_load_dword                           v[3], v[32], s[44:47], 0 offen offset:3696 lds
    buffer_load_dword                           v[3], v[33], s[44:47], 0 offen offset:3960 lds
    s_add_u32                                   s[38], s[38], 1
    s_cmp_eq_i32                                s[38], 3
    s_cselect_b32                               s[37], 0, s[38]
    s_mov_b32                                   s[38], s[37]
    s_mul_i32                                   s[31], s[32], s[37]
    s_add_u32                                   s[29], s[30], s[31]
    s_mul_i32                                   s[35], s[36], s[37]
    s_add_u32                                   s[33], s[34], s[35]
    v_add_u32                                   v[14], 128, v[14]
    v_add_u32                                   v[15], 128, v[15]
    v_add_u32                                   v[16], 128, v[16]
    v_add_u32                                   v[17], 128, v[17]
    v_add_u32                                   v[18], 128, v[18]
    v_add_u32                                   v[19], 128, v[19]
    v_add_u32                                   v[20], 128, v[20]
    v_add_u32                                   v[21], 128, v[21]
    v_add_u32                                   v[22], 128, v[22]
    v_add_u32                                   v[23], 128, v[23]
    v_add_u32                                   v[24], 128, v[24]
    v_add_u32                                   v[25], 128, v[25]
    v_add_u32                                   v[26], 128, v[26]
    v_add_u32                                   v[27], 128, v[27]
    v_add_u32                                   v[28], 128, v[28]
    v_add_u32                                   v[29], 128, v[29]
    v_add_u32                                   v[30], 128, v[30]
    v_add_u32                                   v[31], 128, v[31]
    v_add_u32                                   v[32], 128, v[32]
    v_add_u32                                   v[33], 128, v[33]
    s_sub_u32                                   s[48], s[39], 1
FETCH_LOOP:
    s_mov_b32                                   m0, s[29]
    buffer_load_dword                           v[3], v[14], s[40:43], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[15], s[40:43], 0 offen offset:264 lds
    buffer_load_dword                           v[3], v[16], s[40:43], 0 offen offset:528 lds
    buffer_load_dword                           v[3], v[17], s[40:43], 0 offen offset:792 lds
    s_mov_b32                                   m0, s[33]
    buffer_load_dword                           v[3], v[18], s[44:47], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[19], s[44:47], 0 offen offset:264 lds
    buffer_load_dword                           v[3], v[20], s[44:47], 0 offen offset:528 lds
    buffer_load_dword                           v[3], v[21], s[44:47], 0 offen offset:792 lds
    buffer_load_dword                           v[3], v[22], s[44:47], 0 offen offset:1056 lds
    buffer_load_dword                           v[3], v[23], s[44:47], 0 offen offset:1320 lds
    buffer_load_dword                           v[3], v[24], s[44:47], 0 offen offset:1584 lds
    buffer_load_dword                           v[3], v[25], s[44:47], 0 offen offset:1848 lds
    buffer_load_dword                           v[3], v[26], s[44:47], 0 offen offset:2112 lds
    buffer_load_dword                           v[3], v[27], s[44:47], 0 offen offset:2376 lds
    buffer_load_dword                           v[3], v[28], s[44:47], 0 offen offset:2640 lds
    buffer_load_dword                           v[3], v[29], s[44:47], 0 offen offset:2904 lds
    buffer_load_dword                           v[3], v[30], s[44:47], 0 offen offset:3168 lds
    buffer_load_dword                           v[3], v[31], s[44:47], 0 offen offset:3432 lds
    buffer_load_dword                           v[3], v[32], s[44:47], 0 offen offset:3696 lds
    buffer_load_dword                           v[3], v[33], s[44:47], 0 offen offset:3960 lds
    s_add_u32                                   s[38], s[38], 1
    s_cmp_eq_i32                                s[38], 3
    s_cselect_b32                               s[37], 0, s[38]
    s_mov_b32                                   s[38], s[37]
    s_mul_i32                                   s[31], s[32], s[37]
    s_add_u32                                   s[29], s[30], s[31]
    s_mul_i32                                   s[35], s[36], s[37]
    s_add_u32                                   s[33], s[34], s[35]
    v_add_u32                                   v[14], 128, v[14]
    v_add_u32                                   v[15], 128, v[15]
    v_add_u32                                   v[16], 128, v[16]
    v_add_u32                                   v[17], 128, v[17]
    v_add_u32                                   v[18], 128, v[18]
    v_add_u32                                   v[19], 128, v[19]
    v_add_u32                                   v[20], 128, v[20]
    v_add_u32                                   v[21], 128, v[21]
    v_add_u32                                   v[22], 128, v[22]
    v_add_u32                                   v[23], 128, v[23]
    v_add_u32                                   v[24], 128, v[24]
    v_add_u32                                   v[25], 128, v[25]
    v_add_u32                                   v[26], 128, v[26]
    v_add_u32                                   v[27], 128, v[27]
    v_add_u32                                   v[28], 128, v[28]
    v_add_u32                                   v[29], 128, v[29]
    v_add_u32                                   v[30], 128, v[30]
    v_add_u32                                   v[31], 128, v[31]
    v_add_u32                                   v[32], 128, v[32]
    v_add_u32                                   v[33], 128, v[33]
    s_waitcnt                                   vmcnt(20)
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
    v_accvgpr_write                             acc[8], 0
    v_accvgpr_write                             acc[9], 0
    v_accvgpr_write                             acc[10], 0
    v_accvgpr_write                             acc[11], 0
    v_accvgpr_write                             acc[12], 0
    v_accvgpr_write                             acc[13], 0
    v_accvgpr_write                             acc[14], 0
    v_accvgpr_write                             acc[15], 0
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
    v_lshlrev_b32                               v[13], 2, v[3]
    v_lshlrev_b32                               v[14], 2, v[11]
    v_lshrrev_b32                               v[3], 1, v[12]
    v_mul_lo_u32                                v[15], 8, v[3]
    v_add_u32                                   v[17], v[13], v[14]
    v_add_u32                                   v[17], v[17], v[15]
    v_mov_b32                                   v[19], 4224
    v_mov_b32                                   v[16], v[17]
    v_lshrrev_b32                               v[8], 0, s[26]
    v_lshlrev_b32                               v[9], 5, v[8]
    v_and_b32                                   v[10], v[6], 15
    v_lshrrev_b32                               v[11], 4, v[6]
    v_add_u32                                   v[12], v[9], v[10]
    v_lshlrev_b32                               v[3], 5, v[12]
    v_lshlrev_b32                               v[13], 2, v[3]
    v_lshlrev_b32                               v[14], 2, v[11]
    v_lshrrev_b32                               v[3], 1, v[12]
    v_mul_lo_u32                                v[15], 8, v[3]
    v_add_u32                                   v[21], v[13], v[14]
    v_mov_b32                                   v[3], 0x0+12672
    v_add_u32                                   v[21], v[21], v[3]
    v_add_u32                                   v[21], v[21], v[15]
    v_mov_b32                                   v[23], 16896
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
    s_lshl_b32                                  s[30], s[2], 5
    s_lshl_b32                                  s[29], s[3], 7
    s_lshr_b32                                  s[5], s[26], 0
    v_lshlrev_b32                               v[8], 5, s[5]
    s_and_b32                                   s[5], s[26], 0
    v_lshlrev_b32                               v[9], 5, s[5]
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
    v_add_u32                                   v[28], v[26], v[27]
    s_mov_b32                                   s[36:36], s[6:6]
    s_mov_b32                                   s[37:37], s[7:7]
    s_mov_b32                                   s[38:38], 0x0+2147483648
    s_mov_b32                                   s[39:39], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[17:17]
    s_lshl_b32                                  s[5], s[5], 2
    s_add_u32                                   s[36:36], s[36:36], s[5]
    s_addc_u32                                  s[37:37], s[37:37], 0
    s_lshl_b32                                  s[30], s[2], 5
    s_lshl_b32                                  s[29], s[3], 7
    s_lshr_b32                                  s[5], s[26], 0
    v_lshlrev_b32                               v[8], 5, s[5]
    s_and_b32                                   s[5], s[26], 0
    v_lshlrev_b32                               v[9], 5, s[5]
    v_and_b32                                   v[10], 15, v[6]
    v_lshrrev_b32                               v[11], 4, v[6]
    v_lshlrev_b32                               v[12], 2, v[11]
    v_add3_u32                                  v[13], s[29], v[8], v[10]                
    v_add3_u32                                  v[14], s[30], v[9], v[12]                
    v_mul_lo_u32                                v[3], s[16:16], v[13]
    v_lshlrev_b32                               v[15], 2, v[3]
    v_lshlrev_b32                               v[24], 2, v[14]
    v_lshlrev_b32                               v[3], 4, s[16:16]
    v_lshlrev_b32                               v[25], 2, v[3]
    v_add_u32                                   v[29], v[15], v[24]
    v_add_u32                                   v[30], v[25], v[29]
    buffer_load_dwordx4                         v[32:35], v[27], s[32:35], 0 offen offset:0
    buffer_load_dwordx4                         v[36:39], v[27], s[32:35], 0 offen offset:64
    buffer_load_dwordx4                         v[40:43], v[28], s[32:35], 0 offen offset:0
    buffer_load_dwordx4                         v[44:47], v[28], s[32:35], 0 offen offset:64
    s_waitcnt                                   vmcnt(1)
    v_mul_f32                                   v[32], s[15:15], v[32]
    v_mul_f32                                   v[33], s[15:15], v[33]
    v_mul_f32                                   v[34], s[15:15], v[34]
    v_mul_f32                                   v[35], s[15:15], v[35]
    s_waitcnt                                   vmcnt(0)
    v_mul_f32                                   v[36], s[15:15], v[36]
    v_mul_f32                                   v[37], s[15:15], v[37]
    v_mul_f32                                   v[38], s[15:15], v[38]
    v_mul_f32                                   v[39], s[15:15], v[39]
    s_waitcnt                                   vmcnt(0)
    v_mul_f32                                   v[40], s[15:15], v[40]
    v_mul_f32                                   v[41], s[15:15], v[41]
    v_mul_f32                                   v[42], s[15:15], v[42]
    v_mul_f32                                   v[43], s[15:15], v[43]
    s_waitcnt                                   vmcnt(0)
    v_mul_f32                                   v[44], s[15:15], v[44]
    v_mul_f32                                   v[45], s[15:15], v[45]
    v_mul_f32                                   v[46], s[15:15], v[46]
    v_mul_f32                                   v[47], s[15:15], v[47]
    s_mov_b32                                   s[30], 0
    s_setprio                                   1
    v_mov_b32                                   v[25], 0
    s_barrier
    ds_read_b32                                 v[8], v[16]
    ds_read_b32                                 v[9], v[16]                              offset:2112
    ds_read_b32                                 v[10], v[20]
    ds_read_b32                                 v[11], v[20]                             offset:2112
    s_lshr_b32                                  s[5], 1024, 5
    s_cmp_le_u32                                s[5], 1
    s_cbranch_scc1                              END_MATH_LOOP
    s_sub_u32                                   s[5], s[5], 1
    s_sub_u32                                   s[31], s[5], 1
MATH_LOOP:
    ds_read_b32                                 v[12], v[16]                             offset:16
    ds_read_b32                                 v[13], v[16]                             offset:2128
    ds_read_b32                                 v[14], v[20]                             offset:16
    ds_read_b32                                 v[15], v[20]                             offset:2128
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[10:10], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[10:10], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[8:8], v[11:11], acc[8:11]   
    v_mfma_f32_16x16x4f32                       acc[12:15], v[9:9], v[11:11], acc[12:15] 
    ds_read_b32                                 v[8], v[16]                              offset:32
    ds_read_b32                                 v[9], v[16]                              offset:2144
    ds_read_b32                                 v[10], v[20]                             offset:32
    ds_read_b32                                 v[11], v[20]                             offset:2144
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[12:12], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[13:13], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[12:12], v[15:15], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[13:13], v[15:15], acc[12:15] 
    ds_read_b32                                 v[12], v[16]                             offset:48
    ds_read_b32                                 v[13], v[16]                             offset:2160
    ds_read_b32                                 v[14], v[20]                             offset:48
    ds_read_b32                                 v[15], v[20]                             offset:2160
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[10:10], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[10:10], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[8:8], v[11:11], acc[8:11]   
    v_mfma_f32_16x16x4f32                       acc[12:15], v[9:9], v[11:11], acc[12:15] 
    ds_read_b32                                 v[8], v[16]                              offset:64
    ds_read_b32                                 v[9], v[16]                              offset:2176
    ds_read_b32                                 v[10], v[20]                             offset:64
    ds_read_b32                                 v[11], v[20]                             offset:2176
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[12:12], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[13:13], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[12:12], v[15:15], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[13:13], v[15:15], acc[12:15] 
    ds_read_b32                                 v[12], v[16]                             offset:80
    ds_read_b32                                 v[13], v[16]                             offset:2192
    ds_read_b32                                 v[14], v[20]                             offset:80
    ds_read_b32                                 v[15], v[20]                             offset:2192
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[10:10], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[10:10], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[8:8], v[11:11], acc[8:11]   
    v_mfma_f32_16x16x4f32                       acc[12:15], v[9:9], v[11:11], acc[12:15] 
    ds_read_b32                                 v[8], v[16]                              offset:96
    ds_read_b32                                 v[9], v[16]                              offset:2208
    ds_read_b32                                 v[10], v[20]                             offset:96
    ds_read_b32                                 v[11], v[20]                             offset:2208
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[12:12], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[13:13], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[12:12], v[15:15], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[13:13], v[15:15], acc[12:15] 
    ds_read_b32                                 v[12], v[16]                             offset:112
    ds_read_b32                                 v[13], v[16]                             offset:2224
    ds_read_b32                                 v[14], v[20]                             offset:112
    ds_read_b32                                 v[15], v[20]                             offset:2224
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[10:10], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[10:10], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[8:8], v[11:11], acc[8:11]   
    v_add_u32                                   v[25], v[25], 1
    v_cmp_eq_u32                                vcc, v[25], 3
    v_cndmask_b32                               v[24], v[25], 0, vcc                     
    v_mov_b32                                   v[25], v[24]
    v_mad_u32_u24                               v[16], v[19], v[24], v[17]               
    v_mad_u32_u24                               v[20], v[23], v[24], v[21]               
    v_mfma_f32_16x16x4f32                       acc[12:15], v[9:9], v[11:11], acc[12:15] 
    s_waitcnt                                   lgkmcnt(0)
    s_barrier
    v_mfma_f32_16x16x4f32                       acc[0:3], v[12:12], v[14:14], acc[0:3]   
    ds_read_b32                                 v[8], v[16]
    ds_read_b32                                 v[9], v[16]                              offset:2112
    ds_read_b32                                 v[10], v[20]
    ds_read_b32                                 v[11], v[20]                             offset:2112
    v_mfma_f32_16x16x4f32                       acc[4:7], v[13:13], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[12:12], v[15:15], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[13:13], v[15:15], acc[12:15] 
    s_sub_u32                                   s[31], s[31], 1
    s_cbranch_scc0                              MATH_LOOP
END_MATH_LOOP:
    ds_read_b32                                 v[12], v[16]                             offset:16
    ds_read_b32                                 v[13], v[16]                             offset:2128
    ds_read_b32                                 v[14], v[20]                             offset:16
    ds_read_b32                                 v[15], v[20]                             offset:2128
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[10:10], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[10:10], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[8:8], v[11:11], acc[8:11]   
    v_mfma_f32_16x16x4f32                       acc[12:15], v[9:9], v[11:11], acc[12:15] 
    ds_read_b32                                 v[8], v[16]                              offset:32
    ds_read_b32                                 v[9], v[16]                              offset:2144
    ds_read_b32                                 v[10], v[20]                             offset:32
    ds_read_b32                                 v[11], v[20]                             offset:2144
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[12:12], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[13:13], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[12:12], v[15:15], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[13:13], v[15:15], acc[12:15] 
    ds_read_b32                                 v[12], v[16]                             offset:48
    ds_read_b32                                 v[13], v[16]                             offset:2160
    ds_read_b32                                 v[14], v[20]                             offset:48
    ds_read_b32                                 v[15], v[20]                             offset:2160
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[10:10], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[10:10], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[8:8], v[11:11], acc[8:11]   
    v_mfma_f32_16x16x4f32                       acc[12:15], v[9:9], v[11:11], acc[12:15] 
    ds_read_b32                                 v[8], v[16]                              offset:64
    ds_read_b32                                 v[9], v[16]                              offset:2176
    ds_read_b32                                 v[10], v[20]                             offset:64
    ds_read_b32                                 v[11], v[20]                             offset:2176
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[12:12], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[13:13], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[12:12], v[15:15], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[13:13], v[15:15], acc[12:15] 
    ds_read_b32                                 v[12], v[16]                             offset:80
    ds_read_b32                                 v[13], v[16]                             offset:2192
    ds_read_b32                                 v[14], v[20]                             offset:80
    ds_read_b32                                 v[15], v[20]                             offset:2192
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[10:10], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[10:10], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[8:8], v[11:11], acc[8:11]   
    v_mfma_f32_16x16x4f32                       acc[12:15], v[9:9], v[11:11], acc[12:15] 
    ds_read_b32                                 v[8], v[16]                              offset:96
    ds_read_b32                                 v[9], v[16]                              offset:2208
    ds_read_b32                                 v[10], v[20]                             offset:96
    ds_read_b32                                 v[11], v[20]                             offset:2208
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[12:12], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[13:13], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[12:12], v[15:15], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[13:13], v[15:15], acc[12:15] 
    ds_read_b32                                 v[12], v[16]                             offset:112
    ds_read_b32                                 v[13], v[16]                             offset:2224
    ds_read_b32                                 v[14], v[20]                             offset:112
    ds_read_b32                                 v[15], v[20]                             offset:2224
    s_waitcnt                                   lgkmcnt(4)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[10:10], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[10:10], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[8:8], v[11:11], acc[8:11]   
    v_mfma_f32_16x16x4f32                       acc[12:15], v[9:9], v[11:11], acc[12:15] 
    s_waitcnt                                   lgkmcnt(0)
    v_mov_b32                                   v[26], 0x0+4294901760
    v_mfma_f32_16x16x4f32                       acc[0:3], v[12:12], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[13:13], v[14:14], acc[4:7]   
    v_accvgpr_read                              v[48], acc[0:0]
    s_nop                                       4
    v_fma_f32                                   v[48], s[14:14], v[48], v[32]            
    v_accvgpr_read                              v[49], acc[1:1]
    s_nop                                       4
    v_fma_f32                                   v[49], s[14:14], v[49], v[33]            
    v_accvgpr_read                              v[50], acc[2:2]
    s_nop                                       4
    v_fma_f32                                   v[50], s[14:14], v[50], v[34]            
    v_accvgpr_read                              v[51], acc[3:3]
    s_nop                                       4
    v_fma_f32                                   v[51], s[14:14], v[51], v[35]            
    buffer_store_dwordx4                        v[48:51], v[29], s[36:39], 0 offen offset:0
    v_mfma_f32_16x16x4f32                       acc[8:11], v[12:12], v[15:15], acc[8:11] 
    v_accvgpr_read                              v[48], acc[4:4]
    s_nop                                       4
    v_fma_f32                                   v[48], s[14:14], v[48], v[36]            
    v_accvgpr_read                              v[49], acc[5:5]
    s_nop                                       4
    v_fma_f32                                   v[49], s[14:14], v[49], v[37]            
    v_accvgpr_read                              v[50], acc[6:6]
    s_nop                                       4
    v_fma_f32                                   v[50], s[14:14], v[50], v[38]            
    v_accvgpr_read                              v[51], acc[7:7]
    s_nop                                       4
    v_fma_f32                                   v[51], s[14:14], v[51], v[39]            
    buffer_store_dwordx4                        v[48:51], v[29], s[36:39], 0 offen offset:64
    v_mfma_f32_16x16x4f32                       acc[12:15], v[13:13], v[15:15], acc[12:15] 
    v_accvgpr_read                              v[48], acc[8:8]
    s_nop                                       4
    v_fma_f32                                   v[48], s[14:14], v[48], v[40]            
    v_accvgpr_read                              v[49], acc[9:9]
    s_nop                                       4
    v_fma_f32                                   v[49], s[14:14], v[49], v[41]            
    v_accvgpr_read                              v[50], acc[10:10]
    s_nop                                       4
    v_fma_f32                                   v[50], s[14:14], v[50], v[42]            
    v_accvgpr_read                              v[51], acc[11:11]
    s_nop                                       4
    v_fma_f32                                   v[51], s[14:14], v[51], v[43]            
    buffer_store_dwordx4                        v[48:51], v[30], s[36:39], 0 offen offset:0
    v_accvgpr_read                              v[48], acc[12:12]
    s_nop                                       4
    v_fma_f32                                   v[48], s[14:14], v[48], v[44]            
    v_accvgpr_read                              v[49], acc[13:13]
    s_nop                                       4
    v_fma_f32                                   v[49], s[14:14], v[49], v[45]            
    v_accvgpr_read                              v[50], acc[14:14]
    s_nop                                       4
    v_fma_f32                                   v[50], s[14:14], v[50], v[46]            
    v_accvgpr_read                              v[51], acc[15:15]
    s_nop                                       4
    v_fma_f32                                   v[51], s[14:14], v[51], v[47]            
    buffer_store_dwordx4                        v[48:51], v[30], s[36:39], 0 offen offset:64
END_PROG:
    s_endpgm


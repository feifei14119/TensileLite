.amdgcn_target "amdgcn-amd-amdhsa--gfx908+sram-ecc"
.text
.protected Cijk_Alik_Bljk_SB_MT256x32x16_SE_K1
.globl Cijk_Alik_Bljk_SB_MT256x32x16_SE_K1
.p2align 8
.type Cijk_Alik_Bljk_SB_MT256x32x16_SE_K1,@function
.section .rodata,#alloc
.p2align 6
.amdhsa_kernel Cijk_Alik_Bljk_SB_MT256x32x16_SE_K1
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_next_free_vgpr 72
  .amdhsa_next_free_sgpr 55
  .amdhsa_group_segment_fixed_size 57024
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
  - .name: Cijk_Alik_Bljk_SB_MT256x32x16_SE_K1
    .symbol: 'Cijk_Alik_Bljk_SB_MT256x32x16_SE_K1.kd'
    .language: OpenCL C
    .language_version: [ 2, 0 ]
    .group_segment_fixed_size: 57024
    .kernarg_segment_align: 8
    .kernarg_segment_size: 148
    .max_flat_workgroup_size: 512
    .private_segment_fixed_size: 0
    .sgpr_count: 55
    .sgpr_spill_count: 0
    .vgpr_count: 72
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
Cijk_Alik_Bljk_SB_MT256x32x16_SE_K1:
START_PROG:
    v_and_b32                                   v[6], 63, v[0]
    v_lshrrev_b32                               v[7], 6, v[0]
    v_readfirstlane_b32                         s[26], v[7]
    s_mul_i32                                   s[5], 8, s[3]
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
    s_mov_b32                                   s[5], 4224
    s_mul_i32                                   s[30], s[26], s[5]
    s_mov_b32                                   s[32], 16896
    s_mov_b32                                   s[29], s[30]
    s_mov_b32                                   s[5], 528
    s_mul_i32                                   s[34], s[26], s[5]
    s_mov_b32                                   s[5], 50688
    s_add_u32                                   s[34], s[34], s[5]
    s_mov_b32                                   s[36], 2112
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
    s_lshl_b32                                  s[37], s[2], 8
    v_lshlrev_b32                               v[8], 6, s[26]
    v_lshrrev_b32                               v[9], 4, v[6]
    v_and_b32                                   v[10], 15, v[6]
    v_add3_u32                                  v[11], s[37], v[8], v[9]                 
    v_mul_lo_u32                                v[3], s[20:20], v[11]
    v_lshlrev_b32                               v[12], 2, v[3]
    v_lshlrev_b32                               v[13], 2, v[10]
    s_lshl_b32                                  s[38], s[20:20], 4
    s_sub_u32                                   s[38], s[38], 264
    v_add_u32                                   v[14], v[12], v[13]
    v_add_u32                                   v[15], s[38], v[14]
    v_add_u32                                   v[16], s[38], v[15]
    v_add_u32                                   v[17], s[38], v[16]
    v_add_u32                                   v[18], s[38], v[17]
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
    s_mov_b32                                   s[44:44], s[12:12]
    s_mov_b32                                   s[45:45], s[13:13]
    s_mov_b32                                   s[46:46], 0x0+2147483648
    s_mov_b32                                   s[47:47], 0x0+131072
    s_mul_i32                                   s[5], s[4], s[23:23]
    s_lshl_b32                                  s[5], s[5], 2
    s_add_u32                                   s[44:44], s[44:44], s[5]
    s_addc_u32                                  s[45:45], s[45:45], 0
    s_lshl_b32                                  s[37], s[3], 5
    v_lshlrev_b32                               v[8], 3, s[26]
    v_lshrrev_b32                               v[9], 4, v[6]
    v_and_b32                                   v[10], 15, v[6]
    v_add3_u32                                  v[11], s[37], v[8], v[9]                 
    v_mul_lo_u32                                v[3], s[22:22], v[11]
    v_lshlrev_b32                               v[12], 2, v[3]
    v_lshlrev_b32                               v[13], 2, v[10]
    s_lshl_b32                                  s[38], s[22:22], 4
    s_sub_u32                                   s[38], s[38], 264
    v_add_u32                                   v[30], v[12], v[13]
    v_add_u32                                   v[31], s[38], v[30]
    s_mov_b32                                   s[38], 0
    s_lshr_b32                                  s[39], 480, 4
    s_cmp_le_u32                                s[39], 1
    s_cbranch_scc1                              END_FETCH_LOOP
    s_sub_u32                                   s[39], s[39], 1
    s_mov_b32                                   m0, s[29]
    buffer_load_dword                           v[3], v[14], s[40:43], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[15], s[40:43], 0 offen offset:264 lds
    buffer_load_dword                           v[3], v[16], s[40:43], 0 offen offset:528 lds
    buffer_load_dword                           v[3], v[17], s[40:43], 0 offen offset:792 lds
    buffer_load_dword                           v[3], v[18], s[40:43], 0 offen offset:1056 lds
    buffer_load_dword                           v[3], v[19], s[40:43], 0 offen offset:1320 lds
    buffer_load_dword                           v[3], v[20], s[40:43], 0 offen offset:1584 lds
    buffer_load_dword                           v[3], v[21], s[40:43], 0 offen offset:1848 lds
    buffer_load_dword                           v[3], v[22], s[40:43], 0 offen offset:2112 lds
    buffer_load_dword                           v[3], v[23], s[40:43], 0 offen offset:2376 lds
    buffer_load_dword                           v[3], v[24], s[40:43], 0 offen offset:2640 lds
    buffer_load_dword                           v[3], v[25], s[40:43], 0 offen offset:2904 lds
    buffer_load_dword                           v[3], v[26], s[40:43], 0 offen offset:3168 lds
    buffer_load_dword                           v[3], v[27], s[40:43], 0 offen offset:3432 lds
    buffer_load_dword                           v[3], v[28], s[40:43], 0 offen offset:3696 lds
    buffer_load_dword                           v[3], v[29], s[40:43], 0 offen offset:3960 lds
    s_mov_b32                                   m0, s[33]
    buffer_load_dword                           v[3], v[30], s[44:47], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[31], s[44:47], 0 offen offset:264 lds
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
    v_add_u32                                   v[20], 64, v[20]
    v_add_u32                                   v[21], 64, v[21]
    v_add_u32                                   v[22], 64, v[22]
    v_add_u32                                   v[23], 64, v[23]
    v_add_u32                                   v[24], 64, v[24]
    v_add_u32                                   v[25], 64, v[25]
    v_add_u32                                   v[26], 64, v[26]
    v_add_u32                                   v[27], 64, v[27]
    v_add_u32                                   v[28], 64, v[28]
    v_add_u32                                   v[29], 64, v[29]
    v_add_u32                                   v[30], 64, v[30]
    v_add_u32                                   v[31], 64, v[31]
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
    buffer_load_dword                           v[3], v[22], s[40:43], 0 offen offset:2112 lds
    buffer_load_dword                           v[3], v[23], s[40:43], 0 offen offset:2376 lds
    buffer_load_dword                           v[3], v[24], s[40:43], 0 offen offset:2640 lds
    buffer_load_dword                           v[3], v[25], s[40:43], 0 offen offset:2904 lds
    buffer_load_dword                           v[3], v[26], s[40:43], 0 offen offset:3168 lds
    buffer_load_dword                           v[3], v[27], s[40:43], 0 offen offset:3432 lds
    buffer_load_dword                           v[3], v[28], s[40:43], 0 offen offset:3696 lds
    buffer_load_dword                           v[3], v[29], s[40:43], 0 offen offset:3960 lds
    s_mov_b32                                   m0, s[33]
    buffer_load_dword                           v[3], v[30], s[44:47], 0 offen offset:0 lds
    buffer_load_dword                           v[3], v[31], s[44:47], 0 offen offset:264 lds
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
    v_add_u32                                   v[20], 64, v[20]
    v_add_u32                                   v[21], 64, v[21]
    v_add_u32                                   v[22], 64, v[22]
    v_add_u32                                   v[23], 64, v[23]
    v_add_u32                                   v[24], 64, v[24]
    v_add_u32                                   v[25], 64, v[25]
    v_add_u32                                   v[26], 64, v[26]
    v_add_u32                                   v[27], 64, v[27]
    v_add_u32                                   v[28], 64, v[28]
    v_add_u32                                   v[29], 64, v[29]
    v_add_u32                                   v[30], 64, v[30]
    v_add_u32                                   v[31], 64, v[31]
    s_waitcnt                                   vmcnt(18)
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
    v_accvgpr_write                             acc[16], 0
    v_accvgpr_write                             acc[17], 0
    v_accvgpr_write                             acc[18], 0
    v_accvgpr_write                             acc[19], 0
    v_accvgpr_write                             acc[20], 0
    v_accvgpr_write                             acc[21], 0
    v_accvgpr_write                             acc[22], 0
    v_accvgpr_write                             acc[23], 0
    v_accvgpr_write                             acc[24], 0
    v_accvgpr_write                             acc[25], 0
    v_accvgpr_write                             acc[26], 0
    v_accvgpr_write                             acc[27], 0
    v_accvgpr_write                             acc[28], 0
    v_accvgpr_write                             acc[29], 0
    v_accvgpr_write                             acc[30], 0
    v_accvgpr_write                             acc[31], 0
    s_load_dwordx2                              s[8:9], s[0:1], 32
    s_load_dwordx2                              s[6:7], s[0:1], 24
    s_load_dwordx2                              s[14:15], s[0:1], 56
    s_load_dwordx4                              s[16:19], s[0:1], 64
    v_and_b32                                   v[8], 3, s[26]
    v_lshlrev_b32                               v[9], 6, v[8]
    v_and_b32                                   v[10], v[6], 15
    v_lshrrev_b32                               v[11], 4, v[6]
    v_add_u32                                   v[12], v[9], v[10]
    v_lshlrev_b32                               v[3], 4, v[12]
    v_lshlrev_b32                               v[13], 2, v[3]
    v_lshlrev_b32                               v[14], 2, v[11]
    v_lshrrev_b32                               v[3], 2, v[12]
    v_mul_lo_u32                                v[15], 8, v[3]
    v_add_u32                                   v[17], v[13], v[14]
    v_add_u32                                   v[17], v[17], v[15]
    v_mov_b32                                   v[19], 16896
    v_mov_b32                                   v[16], v[17]
    v_lshrrev_b32                               v[8], 2, s[26]
    v_lshlrev_b32                               v[9], 5, v[8]
    v_and_b32                                   v[10], v[6], 15
    v_lshrrev_b32                               v[11], 4, v[6]
    v_add_u32                                   v[12], v[9], v[10]
    v_lshlrev_b32                               v[3], 4, v[12]
    v_lshlrev_b32                               v[13], 2, v[3]
    v_lshlrev_b32                               v[14], 2, v[11]
    v_lshrrev_b32                               v[3], 2, v[12]
    v_mul_lo_u32                                v[15], 8, v[3]
    v_add_u32                                   v[21], v[13], v[14]
    v_mov_b32                                   v[3], 0x0+50688
    v_add_u32                                   v[21], v[21], v[3]
    v_add_u32                                   v[21], v[21], v[15]
    v_mov_b32                                   v[23], 2112
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
    s_lshl_b32                                  s[30], s[2], 8
    s_lshl_b32                                  s[29], s[3], 5
    s_lshr_b32                                  s[5], s[26], 2
    v_lshlrev_b32                               v[8], 5, s[5]
    s_and_b32                                   s[5], s[26], 3
    v_lshlrev_b32                               v[9], 6, s[5]
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
    s_lshl_b32                                  s[30], s[2], 8
    s_lshl_b32                                  s[29], s[3], 5
    s_lshr_b32                                  s[5], s[26], 2
    v_lshlrev_b32                               v[8], 5, s[5]
    s_and_b32                                   s[5], s[26], 3
    v_lshlrev_b32                               v[9], 6, s[5]
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
    buffer_load_dwordx4                         v[40:43], v[27], s[32:35], 0 offen offset:128
    buffer_load_dwordx4                         v[44:47], v[27], s[32:35], 0 offen offset:192
    buffer_load_dwordx4                         v[48:51], v[28], s[32:35], 0 offen offset:0
    buffer_load_dwordx4                         v[52:55], v[28], s[32:35], 0 offen offset:64
    buffer_load_dwordx4                         v[56:59], v[28], s[32:35], 0 offen offset:128
    buffer_load_dwordx4                         v[60:63], v[28], s[32:35], 0 offen offset:192
    s_waitcnt                                   vmcnt(3)
    v_mul_f32                                   v[32], s[15:15], v[32]
    v_mul_f32                                   v[33], s[15:15], v[33]
    v_mul_f32                                   v[34], s[15:15], v[34]
    v_mul_f32                                   v[35], s[15:15], v[35]
    s_waitcnt                                   vmcnt(2)
    v_mul_f32                                   v[36], s[15:15], v[36]
    v_mul_f32                                   v[37], s[15:15], v[37]
    v_mul_f32                                   v[38], s[15:15], v[38]
    v_mul_f32                                   v[39], s[15:15], v[39]
    s_waitcnt                                   vmcnt(1)
    v_mul_f32                                   v[40], s[15:15], v[40]
    v_mul_f32                                   v[41], s[15:15], v[41]
    v_mul_f32                                   v[42], s[15:15], v[42]
    v_mul_f32                                   v[43], s[15:15], v[43]
    s_waitcnt                                   vmcnt(0)
    v_mul_f32                                   v[44], s[15:15], v[44]
    v_mul_f32                                   v[45], s[15:15], v[45]
    v_mul_f32                                   v[46], s[15:15], v[46]
    v_mul_f32                                   v[47], s[15:15], v[47]
    s_waitcnt                                   vmcnt(0)
    v_mul_f32                                   v[48], s[15:15], v[48]
    v_mul_f32                                   v[49], s[15:15], v[49]
    v_mul_f32                                   v[50], s[15:15], v[50]
    v_mul_f32                                   v[51], s[15:15], v[51]
    s_waitcnt                                   vmcnt(0)
    v_mul_f32                                   v[52], s[15:15], v[52]
    v_mul_f32                                   v[53], s[15:15], v[53]
    v_mul_f32                                   v[54], s[15:15], v[54]
    v_mul_f32                                   v[55], s[15:15], v[55]
    s_waitcnt                                   vmcnt(0)
    v_mul_f32                                   v[56], s[15:15], v[56]
    v_mul_f32                                   v[57], s[15:15], v[57]
    v_mul_f32                                   v[58], s[15:15], v[58]
    v_mul_f32                                   v[59], s[15:15], v[59]
    s_waitcnt                                   vmcnt(0)
    v_mul_f32                                   v[60], s[15:15], v[60]
    v_mul_f32                                   v[61], s[15:15], v[61]
    v_mul_f32                                   v[62], s[15:15], v[62]
    v_mul_f32                                   v[63], s[15:15], v[63]
    s_mov_b32                                   s[30], 0
    s_setprio                                   1
    v_mov_b32                                   v[25], 0
    s_barrier
    ds_read_b32                                 v[8], v[16]
    ds_read_b32                                 v[9], v[16]                              offset:1056
    ds_read_b32                                 v[10], v[16]                             offset:2112
    ds_read_b32                                 v[11], v[16]                             offset:3168
    ds_read_b32                                 v[12], v[20]
    ds_read_b32                                 v[13], v[20]                             offset:1056
    s_lshr_b32                                  s[5], 480, 4
    s_cmp_le_u32                                s[5], 1
    s_cbranch_scc1                              END_MATH_LOOP
    s_sub_u32                                   s[5], s[5], 1
    s_sub_u32                                   s[31], s[5], 1
MATH_LOOP:
    ds_read_b32                                 v[64], v[16]                             offset:16
    ds_read_b32                                 v[65], v[16]                             offset:1072
    ds_read_b32                                 v[66], v[16]                             offset:2128
    ds_read_b32                                 v[67], v[16]                             offset:3184
    ds_read_b32                                 v[14], v[20]                             offset:16
    ds_read_b32                                 v[15], v[20]                             offset:1072
    s_waitcnt                                   lgkmcnt(6)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[12:12], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[12:12], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[10:10], v[12:12], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[11:11], v[12:12], acc[12:15] 
    v_mfma_f32_16x16x4f32                       acc[16:19], v[8:8], v[13:13], acc[16:19] 
    v_mfma_f32_16x16x4f32                       acc[20:23], v[9:9], v[13:13], acc[20:23] 
    v_mfma_f32_16x16x4f32                       acc[24:27], v[10:10], v[13:13], acc[24:27] 
    v_mfma_f32_16x16x4f32                       acc[28:31], v[11:11], v[13:13], acc[28:31] 
    ds_read_b32                                 v[8], v[16]                              offset:32
    ds_read_b32                                 v[9], v[16]                              offset:1088
    ds_read_b32                                 v[10], v[16]                             offset:2144
    ds_read_b32                                 v[11], v[16]                             offset:3200
    ds_read_b32                                 v[12], v[20]                             offset:32
    ds_read_b32                                 v[13], v[20]                             offset:1088
    s_waitcnt                                   lgkmcnt(6)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[64:64], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[65:65], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[66:66], v[14:14], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[67:67], v[14:14], acc[12:15] 
    v_mfma_f32_16x16x4f32                       acc[16:19], v[64:64], v[15:15], acc[16:19] 
    v_mfma_f32_16x16x4f32                       acc[20:23], v[65:65], v[15:15], acc[20:23] 
    v_mfma_f32_16x16x4f32                       acc[24:27], v[66:66], v[15:15], acc[24:27] 
    v_mfma_f32_16x16x4f32                       acc[28:31], v[67:67], v[15:15], acc[28:31] 
    ds_read_b32                                 v[64], v[16]                             offset:48
    ds_read_b32                                 v[65], v[16]                             offset:1104
    ds_read_b32                                 v[66], v[16]                             offset:2160
    ds_read_b32                                 v[67], v[16]                             offset:3216
    ds_read_b32                                 v[14], v[20]                             offset:48
    ds_read_b32                                 v[15], v[20]                             offset:1104
    s_waitcnt                                   lgkmcnt(6)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[12:12], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[12:12], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[10:10], v[12:12], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[11:11], v[12:12], acc[12:15] 
    v_mfma_f32_16x16x4f32                       acc[16:19], v[8:8], v[13:13], acc[16:19] 
    v_mfma_f32_16x16x4f32                       acc[20:23], v[9:9], v[13:13], acc[20:23] 
    v_mfma_f32_16x16x4f32                       acc[24:27], v[10:10], v[13:13], acc[24:27] 
    v_add_u32                                   v[25], v[25], 1
    v_cmp_eq_u32                                vcc, v[25], 3
    v_cndmask_b32                               v[24], v[25], 0, vcc                     
    v_mov_b32                                   v[25], v[24]
    v_mad_u32_u24                               v[16], v[19], v[24], v[17]               
    v_mad_u32_u24                               v[20], v[23], v[24], v[21]               
    v_mfma_f32_16x16x4f32                       acc[28:31], v[11:11], v[13:13], acc[28:31] 
    s_waitcnt                                   lgkmcnt(0)
    s_barrier
    v_mfma_f32_16x16x4f32                       acc[0:3], v[64:64], v[14:14], acc[0:3]   
    ds_read_b32                                 v[8], v[16]
    ds_read_b32                                 v[9], v[16]                              offset:1056
    ds_read_b32                                 v[10], v[16]                             offset:2112
    ds_read_b32                                 v[11], v[16]                             offset:3168
    ds_read_b32                                 v[12], v[20]
    ds_read_b32                                 v[13], v[20]                             offset:1056
    v_mfma_f32_16x16x4f32                       acc[4:7], v[65:65], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[66:66], v[14:14], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[67:67], v[14:14], acc[12:15] 
    v_mfma_f32_16x16x4f32                       acc[16:19], v[64:64], v[15:15], acc[16:19] 
    v_mfma_f32_16x16x4f32                       acc[20:23], v[65:65], v[15:15], acc[20:23] 
    v_mfma_f32_16x16x4f32                       acc[24:27], v[66:66], v[15:15], acc[24:27] 
    v_mfma_f32_16x16x4f32                       acc[28:31], v[67:67], v[15:15], acc[28:31] 
    s_sub_u32                                   s[31], s[31], 1
    s_cbranch_scc0                              MATH_LOOP
END_MATH_LOOP:
    ds_read_b32                                 v[64], v[16]                             offset:16
    ds_read_b32                                 v[65], v[16]                             offset:1072
    ds_read_b32                                 v[66], v[16]                             offset:2128
    ds_read_b32                                 v[67], v[16]                             offset:3184
    ds_read_b32                                 v[14], v[20]                             offset:16
    ds_read_b32                                 v[15], v[20]                             offset:1072
    s_waitcnt                                   lgkmcnt(6)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[12:12], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[12:12], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[10:10], v[12:12], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[11:11], v[12:12], acc[12:15] 
    v_mfma_f32_16x16x4f32                       acc[16:19], v[8:8], v[13:13], acc[16:19] 
    v_mfma_f32_16x16x4f32                       acc[20:23], v[9:9], v[13:13], acc[20:23] 
    v_mfma_f32_16x16x4f32                       acc[24:27], v[10:10], v[13:13], acc[24:27] 
    v_mfma_f32_16x16x4f32                       acc[28:31], v[11:11], v[13:13], acc[28:31] 
    ds_read_b32                                 v[8], v[16]                              offset:32
    ds_read_b32                                 v[9], v[16]                              offset:1088
    ds_read_b32                                 v[10], v[16]                             offset:2144
    ds_read_b32                                 v[11], v[16]                             offset:3200
    ds_read_b32                                 v[12], v[20]                             offset:32
    ds_read_b32                                 v[13], v[20]                             offset:1088
    s_waitcnt                                   lgkmcnt(6)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[64:64], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[65:65], v[14:14], acc[4:7]   
    v_mfma_f32_16x16x4f32                       acc[8:11], v[66:66], v[14:14], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[67:67], v[14:14], acc[12:15] 
    v_mfma_f32_16x16x4f32                       acc[16:19], v[64:64], v[15:15], acc[16:19] 
    v_mfma_f32_16x16x4f32                       acc[20:23], v[65:65], v[15:15], acc[20:23] 
    v_mfma_f32_16x16x4f32                       acc[24:27], v[66:66], v[15:15], acc[24:27] 
    v_mfma_f32_16x16x4f32                       acc[28:31], v[67:67], v[15:15], acc[28:31] 
    ds_read_b32                                 v[64], v[16]                             offset:48
    ds_read_b32                                 v[65], v[16]                             offset:1104
    ds_read_b32                                 v[66], v[16]                             offset:2160
    ds_read_b32                                 v[67], v[16]                             offset:3216
    ds_read_b32                                 v[14], v[20]                             offset:48
    ds_read_b32                                 v[15], v[20]                             offset:1104
    s_waitcnt                                   lgkmcnt(6)
    v_mfma_f32_16x16x4f32                       acc[0:3], v[8:8], v[12:12], acc[0:3]     
    v_mfma_f32_16x16x4f32                       acc[4:7], v[9:9], v[12:12], acc[4:7]     
    v_mfma_f32_16x16x4f32                       acc[8:11], v[10:10], v[12:12], acc[8:11] 
    v_mfma_f32_16x16x4f32                       acc[12:15], v[11:11], v[12:12], acc[12:15] 
    v_mfma_f32_16x16x4f32                       acc[16:19], v[8:8], v[13:13], acc[16:19] 
    v_mfma_f32_16x16x4f32                       acc[20:23], v[9:9], v[13:13], acc[20:23] 
    v_mfma_f32_16x16x4f32                       acc[24:27], v[10:10], v[13:13], acc[24:27] 
    v_mfma_f32_16x16x4f32                       acc[28:31], v[11:11], v[13:13], acc[28:31] 
    s_waitcnt                                   lgkmcnt(0)
    v_mov_b32                                   v[26], 0x0+4294901760
    v_mfma_f32_16x16x4f32                       acc[0:3], v[64:64], v[14:14], acc[0:3]   
    v_mfma_f32_16x16x4f32                       acc[4:7], v[65:65], v[14:14], acc[4:7]   
    v_accvgpr_read                              v[68], acc[0:0]
    s_nop                                       4
    v_fma_f32                                   v[68], s[14:14], v[68], v[32]            
    v_accvgpr_read                              v[69], acc[1:1]
    s_nop                                       4
    v_fma_f32                                   v[69], s[14:14], v[69], v[33]            
    v_accvgpr_read                              v[70], acc[2:2]
    s_nop                                       4
    v_fma_f32                                   v[70], s[14:14], v[70], v[34]            
    v_accvgpr_read                              v[71], acc[3:3]
    s_nop                                       4
    v_fma_f32                                   v[71], s[14:14], v[71], v[35]            
    buffer_store_dwordx4                        v[68:71], v[29], s[36:39], 0 offen offset:0
    v_mfma_f32_16x16x4f32                       acc[8:11], v[66:66], v[14:14], acc[8:11] 
    v_accvgpr_read                              v[68], acc[4:4]
    s_nop                                       4
    v_fma_f32                                   v[68], s[14:14], v[68], v[36]            
    v_accvgpr_read                              v[69], acc[5:5]
    s_nop                                       4
    v_fma_f32                                   v[69], s[14:14], v[69], v[37]            
    v_accvgpr_read                              v[70], acc[6:6]
    s_nop                                       4
    v_fma_f32                                   v[70], s[14:14], v[70], v[38]            
    v_accvgpr_read                              v[71], acc[7:7]
    s_nop                                       4
    v_fma_f32                                   v[71], s[14:14], v[71], v[39]            
    buffer_store_dwordx4                        v[68:71], v[29], s[36:39], 0 offen offset:64
    v_mfma_f32_16x16x4f32                       acc[12:15], v[67:67], v[14:14], acc[12:15] 
    v_accvgpr_read                              v[68], acc[8:8]
    s_nop                                       4
    v_fma_f32                                   v[68], s[14:14], v[68], v[40]            
    v_accvgpr_read                              v[69], acc[9:9]
    s_nop                                       4
    v_fma_f32                                   v[69], s[14:14], v[69], v[41]            
    v_accvgpr_read                              v[70], acc[10:10]
    s_nop                                       4
    v_fma_f32                                   v[70], s[14:14], v[70], v[42]            
    v_accvgpr_read                              v[71], acc[11:11]
    s_nop                                       4
    v_fma_f32                                   v[71], s[14:14], v[71], v[43]            
    buffer_store_dwordx4                        v[68:71], v[29], s[36:39], 0 offen offset:128
    v_mfma_f32_16x16x4f32                       acc[16:19], v[64:64], v[15:15], acc[16:19] 
    v_accvgpr_read                              v[68], acc[12:12]
    s_nop                                       4
    v_fma_f32                                   v[68], s[14:14], v[68], v[44]            
    v_accvgpr_read                              v[69], acc[13:13]
    s_nop                                       4
    v_fma_f32                                   v[69], s[14:14], v[69], v[45]            
    v_accvgpr_read                              v[70], acc[14:14]
    s_nop                                       4
    v_fma_f32                                   v[70], s[14:14], v[70], v[46]            
    v_accvgpr_read                              v[71], acc[15:15]
    s_nop                                       4
    v_fma_f32                                   v[71], s[14:14], v[71], v[47]            
    buffer_store_dwordx4                        v[68:71], v[29], s[36:39], 0 offen offset:192
    v_mfma_f32_16x16x4f32                       acc[20:23], v[65:65], v[15:15], acc[20:23] 
    v_accvgpr_read                              v[68], acc[16:16]
    s_nop                                       4
    v_fma_f32                                   v[68], s[14:14], v[68], v[48]            
    v_accvgpr_read                              v[69], acc[17:17]
    s_nop                                       4
    v_fma_f32                                   v[69], s[14:14], v[69], v[49]            
    v_accvgpr_read                              v[70], acc[18:18]
    s_nop                                       4
    v_fma_f32                                   v[70], s[14:14], v[70], v[50]            
    v_accvgpr_read                              v[71], acc[19:19]
    s_nop                                       4
    v_fma_f32                                   v[71], s[14:14], v[71], v[51]            
    buffer_store_dwordx4                        v[68:71], v[30], s[36:39], 0 offen offset:0
    v_mfma_f32_16x16x4f32                       acc[24:27], v[66:66], v[15:15], acc[24:27] 
    v_accvgpr_read                              v[68], acc[20:20]
    s_nop                                       4
    v_fma_f32                                   v[68], s[14:14], v[68], v[52]            
    v_accvgpr_read                              v[69], acc[21:21]
    s_nop                                       4
    v_fma_f32                                   v[69], s[14:14], v[69], v[53]            
    v_accvgpr_read                              v[70], acc[22:22]
    s_nop                                       4
    v_fma_f32                                   v[70], s[14:14], v[70], v[54]            
    v_accvgpr_read                              v[71], acc[23:23]
    s_nop                                       4
    v_fma_f32                                   v[71], s[14:14], v[71], v[55]            
    buffer_store_dwordx4                        v[68:71], v[30], s[36:39], 0 offen offset:64
    v_mfma_f32_16x16x4f32                       acc[28:31], v[67:67], v[15:15], acc[28:31] 
    v_accvgpr_read                              v[68], acc[24:24]
    s_nop                                       4
    v_fma_f32                                   v[68], s[14:14], v[68], v[56]            
    v_accvgpr_read                              v[69], acc[25:25]
    s_nop                                       4
    v_fma_f32                                   v[69], s[14:14], v[69], v[57]            
    v_accvgpr_read                              v[70], acc[26:26]
    s_nop                                       4
    v_fma_f32                                   v[70], s[14:14], v[70], v[58]            
    v_accvgpr_read                              v[71], acc[27:27]
    s_nop                                       4
    v_fma_f32                                   v[71], s[14:14], v[71], v[59]            
    buffer_store_dwordx4                        v[68:71], v[30], s[36:39], 0 offen offset:128
    v_accvgpr_read                              v[68], acc[28:28]
    s_nop                                       4
    v_fma_f32                                   v[68], s[14:14], v[68], v[60]            
    v_accvgpr_read                              v[69], acc[29:29]
    s_nop                                       4
    v_fma_f32                                   v[69], s[14:14], v[69], v[61]            
    v_accvgpr_read                              v[70], acc[30:30]
    s_nop                                       4
    v_fma_f32                                   v[70], s[14:14], v[70], v[62]            
    v_accvgpr_read                              v[71], acc[31:31]
    s_nop                                       4
    v_fma_f32                                   v[71], s[14:14], v[71], v[63]            
    buffer_store_dwordx4                        v[68:71], v[30], s[36:39], 0 offen offset:192
END_PROG:
    s_endpgm


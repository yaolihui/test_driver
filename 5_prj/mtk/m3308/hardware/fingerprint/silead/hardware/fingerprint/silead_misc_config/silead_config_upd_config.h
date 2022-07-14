PARAM_UPD_ID_FROM(config, 2, SysParam, SLAlg, normal, 0xC8) /* pb config: 0xC8000000 */
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, ialgi,                              pb.threshold.alg_select,                    TYPE_INT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, iMaxEnrollNum,                      pb.threshold.enrol_num,                     TYPE_INT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, iMaxTemplateNum,                    pb.threshold.max_templates_num,             TYPE_INT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, imax_template_size,                 pb.threshold.templates_size,                TYPE_INT32)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, ienroll_quality_threshold,          pb.threshold.enroll_quality_threshold,      TYPE_INT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, ienroll_coverage_threshold,         pb.threshold.enroll_coverage_threshold,     TYPE_INT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, iverify_quality_threshold,          pb.threshold.quality_threshold,             TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, iverify_coverage_threshold,         pb.threshold.coverage_threshold,            TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, skin_threshold,                     pb.threshold.skin_threshold,                TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, artificial_threshold,               pb.threshold.artificial_threshold,          TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, enroll_same_area,                   pb.threshold.samearea_detect,               TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, isamearea_dist,                     pb.threshold.samearea_dist,                 TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, iverifyStart,                       pb.threshold.samearea_start,                TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, idyupdatefast_set,                  pb.threshold.dy_fast,                       TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, isegment_set,                       pb.threshold.segment,                       TYPE_UINT32)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, water_finger_detect,                pb.threshold.water_finger_detect,           TYPE_UINT32)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, shake_coe,                          pb.threshold.shake_coe,                     TYPE_UINT32)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, noise_coe,                          pb.threshold.noise_coe,                     TYPE_UINT32)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, gray_prec,                          pb.threshold.gray_prec,                     TYPE_UINT32)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, water_detect_threshold,             pb.threshold.water_detect_threshold,        TYPE_UINT32)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, fail_threshold,                     pb.threshold.fail_threshold,                TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, spd_flag,                           pb.threshold.spd_flag,                      TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, iverify_far_high_threshold,         pb.threshold.identify_far_threshold,        TYPE_FAR)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, iverify_uptem_threshold,            pb.threshold.update_far_threshold,          TYPE_FAR)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, isamearea_verify_threshold,         pb.threshold.samearea_threshold,            TYPE_FAR)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, isamefinger_verify_threshold,       pb.threshold.samefinger_threshold,          TYPE_FAR)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, verify_epay_threshold,              pb.threshold.identify_epay_threshold,       TYPE_FAR)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, force_up_threshold,                 pb.threshold.force_up_threshold,            TYPE_FAR)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, isamearea_chk_mask,                 pb.threshold.samearea_mask,                 TYPE_UINT32)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, ireserved8,                         pb.threshold.reserved8,                     TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, isamearea_chk_times_pre_step,       pb.threshold.samearea_check_once_num,       TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, isamearea_chk_max_times,            pb.threshold.samearea_check_num_total,      TYPE_UINT16)

PARAM_UPD_ID_FROM(config, 2, SysParam, SLAlg, deadpix, 0xCD) /* deadpix: 0xCD000000 */
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, fingerdetectThreshold,              test.fd_threshold,                          TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, deadpointhardThershold,             test.deadpx_hard_threshold,                 TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, deadpointnormalThreashold,          test.deadpx_norm_threshold,                 TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, scut,                               test.scut,                                  TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, detev_ww,                           test.detev_ww,                              TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, detev_hh,                           test.detev_hh,                              TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, deteline_h,                         test.deteline_h,                            TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, deteline_w,                         test.deteline_w,                            TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, deadpointmax,                       test.deadpx_max,                            TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, badlinemax,                         test.badline_max,                           TYPE_UINT8)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, deadpoint_finger_detect_mode,       test.finger_detect_mode,                    TYPE_UINT16)
PARAM_UPD_ITEM(config, 2, SysParam, SLAlg, NULL, deadpoint_cut,                      test.deadpx_cut,                            TYPE_UINT32)

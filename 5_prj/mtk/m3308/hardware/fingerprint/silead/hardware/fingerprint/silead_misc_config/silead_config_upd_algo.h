PARAM_UPD_ID_FROM(algo, 1, SLAlg, NULL, algo, 0xEF) /* algo: 0xEF << 23 */
PARAM_UPD_ITEM(algo, 1, SLAlg, NULL,        NULL,           data_ver,       data_ver,                                           TYPE_INT32)
PARAM_UPD_ITEM(algo, 1, SLAlg, NULL,        NULL,           flag,           flag,                                               TYPE_INT32)
PARAM_UPD_ITEM(algo, 2, SLAlg, debase,      NULL,           check_bit,      debase.check_bit,                                   TYPE_INT32)
PARAM_UPD_ITEM(algo, 2, SLAlg, debase,      NULL,           upd_date,       debase.date_upd,                                    TYPE_INT32)
PARAM_UPD_ITEM(algo, 2, SLAlg, debase,      NULL,           data,           debase.int_array,                                   TYPE_TI_ARRAY)
PARAM_UPD_ITEM(algo, 1, SLAlg, NULL,        NULL,           neural_net,     neural_net,                                         TYPE_PARAM_ARRAY)
PARAM_UPD_ITEM(algo, 1, SLAlg, NULL,        NULL,           ext_param,      ext_param,                                          TYPE_PARAM_ARRAY)

#include <iit/ecat/advr/mc_lowpwr_esc.h>
#include <string>

using namespace iit::ecat;
using namespace iit::ecat::advr;

static char acName1000[] = "Device Type";
static char acName1000_0[] = "Device Type";
static char acName1008[] = "Manufacturer Device Name";
static char acName1008_0[] = "Manufacturer Device Name";
static char acName1009[] = "Manufacturer Hardware Version";
static char acName1009_0[] = "Manufacturer Hardware Version";
static char acName100A[] = "Manufacturer Software Version";
static char acName100A_0[] = "Manufacturer Software Version";
static char acName1018[] = "Identity Object";
static char acName1018_0[] = "Number of Elements";
static char acName1018_1[] = "Vendor ID";
static char acName1018_2[] = "Product Code";
static char acName1018_3[] = "Revision Number";
static char acName1018_4[] = "Serial Number";
static char acName1600[] = "Receive PDO Mapping";
static char acName1600_0[] = "Number of Elements";
static char acName1600_n[] = "Mapped Object";
static char acName1A00[] = "Transmit PDO Mapping";
static char acName1A00_0[] = "Number of Elements";
static char acName1A00_n[] = "Mapped Object";
static char acName1C00[] = "Sync Manager Communication Type";
static char acName1C00_0[] = "Number of Elements";
static char acName1C00_1[] = "Communications Type SM0";
static char acName1C00_2[] = "Communications Type SM1";
static char acName1C00_3[] = "Communications Type SM2";
static char acName1C00_4[] = "Communications Type SM3";
static char acName1C12[] = "Sync Manager 2 PDO Assignment";
static char acName1C12_0[] = "Number of Elements";
static char acName1C12_1[] = "PDO Mapping";
static char acName1C13[] = "Sync Manager 3 PDO Assignment";
static char acName1C13_0[] = "Number of Elements";
static char acName1C13_1[] = "PDO Mapping";

static char acName6000[] = "Inputs";
static char acName6000_0[] = "Number of Elements";
static char acName6000_1[] = "rel_enc";
static char acName6000_2[] = "abs_enc";
static char acName6000_3[] = "adc";
static char acName6000_4[] = "ain";
static char acName6000_5[] = "hall";
static char acName6000_6[] = "pwm";
static char acName6000_rtt[] = "rtt";
static char acName6000_pid_out[] = "pid_out";
static char acName6000_pos[] = "position";
static char acName6000_vel[] = "velocity";
static char acName6000_tor[] = "torque";
static char acName6000_tor_D[] = "tor_D";
static char acName6000_curr[] = "curr";
static char acName6000_temp[] = "temperature";
static char acName6000_fault[] = "fault";

static char acName7000[] = "Outputs";
static char acName7000_0[] = "Number of Elements";
static char acName7000_1[] = "pos_ref";
static char acName7000_2[] = "tor_offs";
static char acName7000_3[] = "PGain";
static char acName7000_4[] = "IGain";
static char acName7000_5[] = "DGain";
static char acName7000_6[] = "ts";

static char acName8000[] = "Flash Parameter";
static char acName8000_0[] = "Number of Elements";
static char acName8000_1[] = "Block control";
static char acName8000_2[] = "nonius offset low";
static char acName8000_3[] = "pos_gain_P";
static char acName8000_4[] = "pos_gain_I";
static char acName8000_5[] = "pos_gain_D";
static char acName8000_6[] = "tor_gain_P";
static char acName8000_7[] = "tor_gain_I";
static char acName8000_8[] = "tor_gain_D";
static char acName8000_9[] = "Torque_Mult";
static char acName8000_10[] = "pos_integral_limit";
static char acName8000_11[] = "tor_integral_limit";
static char acName8000_12[] = "Min_pos";
static char acName8000_13[] = "Max_pos";
static char acName8000_14[] = "nonius offset high";
static char acName8000_15[] = "max_tor";
static char acName8000_16[] = "max_cur";
static char acName8000_17[] = "Enc_offset_1";
static char acName8000_18[] = "Enc_offset_2";
static char acName8000_19[] = "Torque_Offset";
static char acName8000_20[] = "ConfigFlags";
static char acName8000_21[] = "ConfigFlags2";
static char acName8000_22[] = "NumEncoderLines";
static char acName8000_23[] = "ImpedancePosGainP";
static char acName8000_24[] = "nonius offset2 low";
static char acName8000_25[] = "ImpedancePosGainD";
static char acName8000_26[] = "Num_Abs_counts_rev";
static char acName8000_27[] = "MaxPWM";
static char acName8000_28[] = "Gearbox_ratio";
static char acName8000_29[] = "ulCalPosition";
static char acName8000_30[] = "Cal_Abs_Position";
static char acName8000_31[] = "Cal_Abs2_Position";
static char acName8000_32[] = "nonius offset2 high";
static char acName8000_33[] = "Joint_number";
static char acName8000_34[] = "Joint_robot_id";
static char acName8000_35[] = "Target_velocity";

static char acName8001[] = "Parameter";
static char acName8001_1[] = "fw_ver";
static char acName8001_2[] = "ack_board_faults";
static char acName8001_3[] = "ctrl_status_cmd";
static char acName8001_4[] = "ctrl_status_cmd_ack";
static char acName8001_5[] = "Direct_ref";
static char acName8001_6[] = "Abs_Pos";
static char acName8001_7[] = "M_Current";
static char acName8001_8[] = "flash_params_cmd";
static char acName8001_9[] = "flash_params_cmd_ack";


static const iit::ecat::objd_t source_SDOs[] = {

    // SD0 0x6000
    { 0x6000, 1, DTYPE_REAL32,          32, ATYPE_RO, "link_pos",       0},
    { 0x6000, 2, DTYPE_REAL32,          32, ATYPE_RO, "motor_pos",      0},
    { 0x6000, 3, DTYPE_REAL32,          32, ATYPE_RO, "link_vel",       0},
    { 0x6000, 4, DTYPE_INTEGER16,       16, ATYPE_RO, "motor_vel",      0},
    { 0x6000, 5, DTYPE_INTEGER16,       16, ATYPE_RO, "torque",     0},
    { 0x6000, 6, DTYPE_UNSIGNED16,      16, ATYPE_RO, "temperature",        0},
    { 0x6000, 7, DTYPE_UNSIGNED16,      16, ATYPE_RO, "fault",      0},
    { 0x6000, 8, DTYPE_UNSIGNED16,      16, ATYPE_RO, "rtt",        0},
    { 0x6000, 9, DTYPE_UNSIGNED16,      16, ATYPE_RO, "op_idx_ack",     0},
    { 0x6000, 10, DTYPE_REAL32,         32, ATYPE_RO, "aux",        0},
    // SD0 0x7000
    { 0x7000, 1, DTYPE_REAL32,          32, ATYPE_RW, "pos_ref",        0},
    { 0x7000, 2, DTYPE_INTEGER16,       16, ATYPE_RW, "vel_ref",        0},
    { 0x7000, 3, DTYPE_INTEGER16,       16, ATYPE_RW, "tor_ref",        0},
    { 0x7000, 4, DTYPE_UNSIGNED16,      16, ATYPE_RW, "gain_0",      0},
    { 0x7000, 5, DTYPE_UNSIGNED16,      16, ATYPE_RW, "gain_1",      0},
    { 0x7000, 6, DTYPE_UNSIGNED16,      16, ATYPE_RW, "gain_2",      0},
    { 0x7000, 7, DTYPE_UNSIGNED16,      16, ATYPE_RW, "gain_3",      0},
    { 0x7000, 8, DTYPE_UNSIGNED16,      16, ATYPE_RW, "gain_4",        0},
    { 0x7000, 9, DTYPE_UNSIGNED16,      16, ATYPE_RW, "fault_ack",      0},
    { 0x7000, 10, DTYPE_UNSIGNED16,     16, ATYPE_RW, "ts",     0},
    { 0x7000, 11, DTYPE_UNSIGNED16,     16, ATYPE_RW, "op_idx_aux",     0},
    { 0x7000, 12, DTYPE_REAL32,         32, ATYPE_RW, "aux",        0},
    // SDO 0x8000
    {0x8000, 0x1, DTYPE_INTEGER32,     32, ATYPE_RW, acName8000_1         ,0   },
    {0x8000, 0x2, DTYPE_INTEGER32,     32, ATYPE_RW, acName8000_2         ,0   },
    {0x8000, 0x3, DTYPE_REAL32,        32, ATYPE_RW, acName8000_3         ,0   },
    {0x8000, 0x4, DTYPE_REAL32,        32, ATYPE_RW, acName8000_4         ,0   },
    {0x8000, 0x5, DTYPE_REAL32,        32, ATYPE_RW, acName8000_5         ,0   },
    {0x8000, 0x6, DTYPE_REAL32,        32, ATYPE_RW, acName8000_6         ,0   },
    {0x8000, 0x7, DTYPE_REAL32,        32, ATYPE_RW, acName8000_7         ,0   },
    {0x8000, 0x8, DTYPE_REAL32,        32, ATYPE_RW, acName8000_8         ,0   },
    {0x8000, 0x9, DTYPE_REAL32,        32, ATYPE_RW, acName8000_9         ,0   },
    {0x8000, 0xa, DTYPE_REAL32,        32, ATYPE_RW, acName8000_10        ,0   },
    {0x8000, 0xb, DTYPE_REAL32,        32, ATYPE_RW, acName8000_11        ,0   },
    {0x8000, 0xc, DTYPE_REAL32,        32, ATYPE_RW, acName8000_12        ,0   },
    {0x8000, 0xd, DTYPE_REAL32,        32, ATYPE_RW, acName8000_13        ,0   },
    {0x8000, 0xe, DTYPE_INTEGER32,     32, ATYPE_RW, acName8000_14        ,0   },
    {0x8000, 0xf, DTYPE_REAL32,        32, ATYPE_RW, acName8000_15        ,0   },
    {0x8000, 0x10, DTYPE_REAL32,       32, ATYPE_RW, acName8000_16        ,0   },
    {0x8000, 0x11, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_17        ,0   },
    {0x8000, 0x12, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_18        ,0   },
    {0x8000, 0x13, DTYPE_REAL32,       32, ATYPE_RW, acName8000_19        ,0   },
    {0x8000, 0x14, DTYPE_INTEGER16,    16, ATYPE_RW, acName8000_20        ,0   },
    {0x8000, 0x15, DTYPE_INTEGER16,    16, ATYPE_RW, acName8000_21        ,0   },
    {0x8000, 0x16, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_22        ,0   },
    {0x8000, 0x17, DTYPE_REAL32,       32, ATYPE_RW, acName8000_23        ,0   },
    {0x8000, 0x18, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_24        ,0   },
    {0x8000, 0x19, DTYPE_REAL32,       32, ATYPE_RW, acName8000_25        ,0   },
    {0x8000, 0x1a, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_26        ,0   },
    {0x8000, 0x1b, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_27        ,0   },
    {0x8000, 0x1c, DTYPE_REAL32,       32, ATYPE_RW, acName8000_28        ,0   },
    {0x8000, 0x1d, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_29        ,0   },
    {0x8000, 0x1e, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_30        ,0   },
    {0x8000, 0x1f, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_31        ,0   },
    {0x8000, 0x20, DTYPE_INTEGER32,    32, ATYPE_RW, acName8000_32        ,0   },
    {0x8000, 0x21, DTYPE_INTEGER16,    16, ATYPE_RW, acName8000_33        ,0   },
    {0x8000, 0x22, DTYPE_INTEGER16,    16, ATYPE_RW, acName8000_34        ,0   },
    {0x8000, 0x23, DTYPE_REAL32,       32, ATYPE_RW, acName8000_35        ,0   },
    
    { 0x8000, 0x24, DTYPE_REAL32,       32, ATYPE_RW,    "Motor_Inertia"            ,0  },
    { 0x8000, 0x25, DTYPE_REAL32,       32, ATYPE_RW,    "Inv_Motor_Inertia"        ,0  },
    { 0x8000, 0x26, DTYPE_REAL32,       32, ATYPE_RW,    "Observer_Cut_Off"         ,0  },
    { 0x8000, 0x27, DTYPE_REAL32,       32, ATYPE_RW,    "Inv_Geared_Torque_Constant",0 },
    { 0x8000, 0x28, DTYPE_REAL32,       32, ATYPE_RW,    "Geared_Torque_Constant"   ,0  },
    { 0x8000, 0x29, DTYPE_REAL32,       32, ATYPE_RW,    "Winding_Resistance"       ,0  },
    { 0x8000, 0x2a, DTYPE_REAL32,       32, ATYPE_RW,    "Voltage_Feedforward"      ,0  },
    { 0x8000, 0x2b, DTYPE_REAL32,       32, ATYPE_RW,    "BackEmf_Compensation"     ,0  },
    
    // SDO 0x8001
    {0x8001, 0x1, DTYPE_VISIBLE_STRING,   64, ATYPE_RO, "fw_ver"                ,0   },
    {0x8001, 0x2, DTYPE_UNSIGNED32,       32, ATYPE_RW, acName8001_2            ,0   },
    {0x8001, 0x3, DTYPE_UNSIGNED16,       16, ATYPE_RW, "ctrl_status_cmd"       ,0   },
    {0x8001, 0x4, DTYPE_UNSIGNED16,       16, ATYPE_RO, "ctrl_status_cmd_ack"   ,0   },
    {0x8001, 0x5, DTYPE_REAL32,           32, ATYPE_RO, acName8001_5            ,0   },
    {0x8001, 0x6, DTYPE_REAL32,           32, ATYPE_RO, acName8001_6            ,0   },
    {0x8001, 0x7, DTYPE_REAL32,           32, ATYPE_RO, acName8001_7            ,0   },
    {0x8001, 0x8, DTYPE_UNSIGNED16,       16, ATYPE_RW, "flash_params_cmd"      ,0   },
    {0x8001, 0x9, DTYPE_UNSIGNED16,       16, ATYPE_RO, "flash_params_cmd_ack"  ,0   },

    {0, 0, 0, 0, 0, 0, 0 }
};





void LpESC::init_SDOs ( void ) {

    int objd_num, i = 0;

    objd_num = sizeof ( source_SDOs ) /sizeof ( objd_t );
    SDOs = new objd_t [objd_num];

    memcpy ( ( void* ) SDOs, source_SDOs, sizeof ( source_SDOs ) );

    //0x6000
    SDOs[i++].data = (void*)&LpESC::rx_pdo.link_pos;
    SDOs[i++].data = (void*)&LpESC::rx_pdo.motor_pos;
    SDOs[i++].data = (void*)&LpESC::rx_pdo.link_vel;
    SDOs[i++].data = (void*)&LpESC::rx_pdo.motor_vel;
    SDOs[i++].data = (void*)&LpESC::rx_pdo.torque;
    SDOs[i++].data = (void*)&LpESC::rx_pdo.temperature;
    SDOs[i++].data = (void*)&LpESC::rx_pdo.fault;
    SDOs[i++].data = (void*)&LpESC::rx_pdo.rtt;
    SDOs[i++].data = (void*)&LpESC::rx_pdo.op_idx_ack;
    SDOs[i++].data = (void*)&LpESC::rx_pdo.aux;

    //0x7000
    SDOs[i++].data = (void*)&LpESC::tx_pdo.pos_ref;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.vel_ref;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.tor_ref;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.gain_0;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.gain_1;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.gain_2;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.gain_3;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.gain_4;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.fault_ack;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.ts;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.op_idx_aux;
    SDOs[i++].data = (void*)&LpESC::tx_pdo.aux;
    
    // 0x8000
    SDOs[i++].data = ( void* ) &LpESC::sdo.Block_control;
    SDOs[i++].data = ( void* ) &LpESC::sdo.nonius_offset_low;
    SDOs[i++].data = ( void* ) &LpESC::sdo.PosGainP;
    SDOs[i++].data = ( void* ) &LpESC::sdo.PosGainI;
    SDOs[i++].data = ( void* ) &LpESC::sdo.PosGainD;
    SDOs[i++].data = ( void* ) &LpESC::sdo.TorGainP;
    SDOs[i++].data = ( void* ) &LpESC::sdo.TorGainI;
    SDOs[i++].data = ( void* ) &LpESC::sdo.TorGainD;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Torque_Mult;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Pos_I_lim;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Tor_I_lim;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Min_pos;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Max_pos;
    SDOs[i++].data = ( void* ) &LpESC::sdo.nonius_offset_high;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Max_tor;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Max_cur;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Enc_offset_1;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Enc_offset_2;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Torque_Offset;
    SDOs[i++].data = ( void* ) &LpESC::sdo.ConfigFlags;
    SDOs[i++].data = ( void* ) &LpESC::sdo.ConfigFlags2;
    SDOs[i++].data = ( void* ) &LpESC::sdo.NumEncoderLines;
    SDOs[i++].data = ( void* ) &LpESC::sdo.ImpedancePosGainP;
    SDOs[i++].data = ( void* ) &LpESC::sdo.nonius_offset2_low;
    SDOs[i++].data = ( void* ) &LpESC::sdo.ImpedancePosGainD;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Num_Abs_counts_rev;
    SDOs[i++].data = ( void* ) &LpESC::sdo.MaxPWM;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Gearbox_ratio;
    SDOs[i++].data = ( void* ) &LpESC::sdo.ulCalPosition;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Cal_Abs_Position;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Cal_Abs2_Position;
    SDOs[i++].data = ( void* ) &LpESC::sdo.nonius_offset2_high;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Joint_number;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Joint_robot_id;
    SDOs[i++].data = ( void* ) &LpESC::sdo.Target_velocity;
    SDOs[i++].data = ( void* ) &LpESC::sdo.MotorInertia;
    SDOs[i++].data = ( void* ) &LpESC::sdo.InvMotorInertia;
    SDOs[i++].data = ( void* ) &LpESC::sdo.ObserverCutOff;
    SDOs[i++].data = ( void* ) &LpESC::sdo.InvGearedTorqueConstant;
    SDOs[i++].data = ( void* ) &LpESC::sdo.GearedTorqueConstant;
    SDOs[i++].data = ( void* ) &LpESC::sdo.WindingResistance;
    SDOs[i++].data = ( void* ) &LpESC::sdo.VoltageFeedforward;
    SDOs[i++].data = ( void* ) &LpESC::sdo.BackEmfCompensation;
    
    // 0x8001
    SDOs[i++].data = ( void* ) &LpESC::sdo.firmware_version;
    SDOs[i++].data = ( void* ) &LpESC::sdo.enable_pdo_gains;
    SDOs[i++].data = ( void* ) &LpESC::sdo.set_ctrl_status;
    SDOs[i++].data = ( void* ) &LpESC::sdo.get_ctrl_status;
    SDOs[i++].data = ( void* ) &LpESC::sdo.V_batt_filt_100ms;
    SDOs[i++].data = ( void* ) &LpESC::sdo.T_inv_filt_100ms;
    SDOs[i++].data = ( void* ) &LpESC::sdo.T_mot1_filt_100ms;
    SDOs[i++].data = ( void* ) &LpESC::sdo.flash_params_cmd;
    SDOs[i++].data = ( void* ) &LpESC::sdo.flash_params_cmd_ack;
    // end marker
    SDOs[i++].data = 0;

    assert ( objd_num == i );
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

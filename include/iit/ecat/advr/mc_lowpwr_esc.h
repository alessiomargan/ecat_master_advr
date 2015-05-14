/*
 * mc_hipwr_esc.h
 * 
 *  HiPower Motor Controlleer 
 *  based on TI TMS320F28335 - Delfino Microcontroller
 *  High-Performance 32-Bit CPU 150 Mhz
 *  
 *  http://www.ti.com/product/tms320f28335
 *  
 *  Created on: Dec 2014
 *      Author: alessio margan
 */

#ifndef __IIT_ECAT_ADVR_MC_LOWPWR_ESC_H__
#define __IIT_ECAT_ADVR_MC_LOWPWR_ESC_H__

#include <iit/ecat/slave_wrapper.h>
#include <iit/ecat/advr/esc.h>
#include <iit/ecat/advr/motor_iface.h>
#include <iit/ecat/advr/log_esc.h>
#include <iit/ecat/advr/pipes.h>
#include <iit/ecat/utils.h>
#include <map>

namespace iit {
namespace ecat {
namespace advr {

namespace lopwr_esc {
//static float J2M(float p, int s, float o) { return ((s*o) + (s*p)); } 
//static float M2J(float p, int s, float o) { return ((p + (s*o))/s); }
static float J2M(float p, int s, float o) { return p; } 
static float M2J(float p, int s, float o) { return p; } 

}

struct LoPwrEscSdoTypes {
    
    // flash

    int Block_control;
    int nonius_offset_low;
    float PosGainP;
    float PosGainI;
    float PosGainD;
    float TorGainP;
    float TorGainI;
    float TorGainD;
    float Torque_Mult;
    float Pos_I_lim;
    float Tor_I_lim;
    float Min_pos;
    float Max_pos;
    int nonius_offset_high;
    float Max_tor;
    float Max_cur;
    int Enc_offset_1;
    int Enc_offset_2;
    float Torque_Offset;
    int16_t ConfigFlags;
    int16_t ConfigFlags2;
    int NumEncoderLines;
    float ImpedancePosGainP;
    int nonius_offset2_low;
    float ImpedancePosGainD;
    int Num_Abs_counts_rev;
    int MaxPWM;
    float Gearbox_ratio;
    int ulCalPosition;
    int Cal_Abs_Position;
    int Cal_Abs2_Position;
    int nonius_offset2_high;

    int16_t     Joint_number;
    int16_t     Joint_robot_id;

    // ram

    char        firmware_version[8];
    uint16_t    enable_pdo_gains;
    uint16_t    set_ctrl_status;
    uint16_t    get_ctrl_status;
    float       V_batt_filt_100ms;
    float       T_inv_filt_100ms;
    float       T_mot1_filt_100ms;
    uint16_t    flash_params_cmd;
    uint16_t    flash_params_cmd_ack;     
};

/**
 *  
 **/ 

class LpESC :
    public BasicEscWrapper<McEscPdoTypes,LoPwrEscSdoTypes>,
    public PDO_log<McEscPdoTypes::pdo_rx>,
    public XDDP_pipe<McEscPdoTypes::pdo_rx,McEscPdoTypes::pdo_tx>,
    public Motor
{
public:
    typedef BasicEscWrapper<McEscPdoTypes,LoPwrEscSdoTypes> Base;
    typedef PDO_log<McEscPdoTypes::pdo_rx>                   Log;
    typedef XDDP_pipe<McEscPdoTypes::pdo_rx,McEscPdoTypes::pdo_tx> Xddp;

    LpESC(const ec_slavet& slave_descriptor) :
        Base(slave_descriptor),
        Log(std::string("/tmp/LpESC_pos"+std::to_string(position)+"_log.txt"),DEFAULT_LOG_SIZE),
        Xddp()
    {

    }
    virtual ~LpESC(void) { 
        delete [] SDOs;
        DPRINTF("~%s %d\n", typeid(this).name(), position);
        print_stat(s_rtt);
    }

    int16_t get_robot_id() {
        //assert(sdo.Joint_robot_id != -1);
        return sdo.Joint_robot_id;
    }

    void print_info(void) {
        DPRINTF("\tJoint id %d\tJoint robot id %d\n", sdo.Joint_number, sdo.Joint_robot_id);
        DPRINTF("\tmin pos %f\tmax pos %f\n", sdo.Min_pos, sdo.Max_pos);
        DPRINTF("\tfw_ver %s\n", sdo.firmware_version);
    }

    virtual const objd_t * get_SDOs() { return SDOs; }

    virtual void init_SDOs(void);

    virtual void on_readPDO(void) {

        if ( rx_pdo.rtt ) {
            rx_pdo.rtt =  (uint16_t)(get_time_ns()/1000000) - rx_pdo.rtt;
            s_rtt(rx_pdo.rtt);
        }

        if ( rx_pdo.fault & 0x7FFF) {
            handle_fault();
        }

        // apply transformation from Motor to Joint 
        rx_pdo.link_pos = lopwr_esc::M2J(rx_pdo.link_pos,_sgn,_offset); 
        rx_pdo.motor_pos = lopwr_esc::M2J(rx_pdo.motor_pos,_sgn,_offset); 
        rx_pdo.pos_ref_fb  = lopwr_esc::M2J(rx_pdo.pos_ref_fb,_sgn,_offset);

        if ( _start_log ) {
            push_back(rx_pdo);
        }

        xddp_write(rx_pdo);
    }

    virtual void on_writePDO(void) {

        tx_pdo.ts = (uint16_t)(get_time_ns()/1000000);

        // apply transformation from Joint to Motor 
        //tx_pdo.pos_ref = J2M(tx_pdo.pos_ref,_sgn,_offset);
    }

    virtual int on_readSDO(const objd_t * sdobj)  {

        if ( ! strcmp(sdobj->name, "link_pos") ) {
            rx_pdo.link_pos = lopwr_esc::M2J(rx_pdo.link_pos,_sgn,_offset);
            //DPRINTF("on_getSDO M2J link_pos %f\n", rx_pdo.position);
        } else if ( ! strcmp(sdobj->name, "motor_pos") ) {
            rx_pdo.motor_pos = lopwr_esc::M2J(rx_pdo.motor_pos,_sgn,_offset);
        } else if ( ! strcmp(sdobj->name, "Min_pos") ) {
            sdo.Min_pos = lopwr_esc::M2J(sdo.Min_pos,_sgn,_offset);
        } else if ( ! strcmp(sdobj->name, "Max_pos") ) {
            sdo.Max_pos = lopwr_esc::M2J(sdo.Max_pos,_sgn,_offset);
        }
        return EC_BOARD_OK;
    }

    virtual int on_writeSDO(const objd_t * sdo) {

        // do not allow to write sdo that map txPDO
        //if ( _actual_state == EC_STATE_OPERATIONAL && sdo->index == 0x7000 ) {
        //    return EC_WRP_SDO_WRITE_CB_FAIL;
        //}
        if ( ! strcmp(sdo->name, "pos_ref") ) {
            tx_pdo.pos_ref = lopwr_esc::J2M(tx_pdo.pos_ref,_sgn,_offset);
            //DPRINTF("on_setSDO J2M pos_ref %f\n", tx_pdo.pos_ref);
        }
        return EC_BOARD_OK;
    }

    ///////////////////////////////////////////////////////
    ///
    /// Motor method implementation
    ///
    ///////////////////////////////////////////////////////
    virtual bool am_i_HpESC() { return false; }
    virtual bool am_i_LpESC() { return true; }
    virtual uint16_t get_ESC_type() { return LO_PWR_DC_MC; }

    virtual const pdo_rx_t& getRxPDO() const {
        return Base::getRxPDO();
    }
    virtual const pdo_tx_t& getTxPDO() const {
        return Base::getTxPDO();
    }
    virtual void setTxPDO(const pdo_tx_t & pdo_tx) {
        Base::setTxPDO(pdo_tx);
    }

    virtual int init(const YAML::Node & root_cfg) {

        Joint_robot_id = -1;

        try {
            // !! sgn and offset must set before init_sdo_lookup !!
            init_SDOs();
            init_sdo_lookup();
            getSDO_byname("Joint_robot_id", Joint_robot_id);

        } catch (EscWrpError &e ) {

            DPRINTF("Catch Exception in %s ... %s\n", __PRETTY_FUNCTION__, e.what());
            return EC_BOARD_INIT_SDO_FAIL;
        }

        if ( Joint_robot_id > 0 ) {
            try {
                std::string esc_conf_key = std::string("LpESC_"+std::to_string(Joint_robot_id));
                if ( root_cfg[esc_conf_key] ) {
                    _sgn = root_cfg[esc_conf_key]["sign"].as<int>(); 
                    _offset = root_cfg[esc_conf_key]["pos_offset"].as<float>();
                    _offset = DEG2RAD(_offset);
                } else {
                    DPRINTF("NO config for  %s in %s\n", esc_conf_key.c_str(), __PRETTY_FUNCTION__);
                    return EC_BOARD_KEY_NOT_FOUND;
                }
            } catch (YAML::KeyNotFound &e) {
                DPRINTF("Catch Exception in %s ... %s\n", __PRETTY_FUNCTION__, e.what());
                return EC_BOARD_KEY_NOT_FOUND;
            }
        } else {
            return EC_BOARD_INVALID_ROBOT_ID;
        }

        // redo read SDOs so we can apply _sgn and _offset to transform Min_pos Max_pos to Joint Coordinate 
        readSDO_byname("Min_pos");
        readSDO_byname("Max_pos");
        readSDO_byname("link_pos");
        
        log_filename = std::string("/tmp/LpESC_"+std::to_string(sdo.Joint_robot_id)+"_log.txt");
        Xddp::init(std::string("LpESC_"+std::to_string(sdo.Joint_robot_id)));
    
        // we log when receive PDOs
        start_log(true);
            
        return EC_WRP_OK;

    }
    virtual int start(int controller_type, float _p, float _i, float _d) {
        
        float act_position, test_ref;
        int32_t fault;

        try {
            set_ctrl_status_X(this, CTRL_POWER_MOD_OFF);
            // set direct mode and power on modulator
            set_ctrl_status_X(this, CTRL_SET_DIRECT_MODE);
            set_ctrl_status_X(this, CTRL_POWER_MOD_ON);
            
            // set position mode
            set_ctrl_status_X(this, CTRL_SET_POS_MODE);
            
        } catch (EscWrpError &e ) {

            DPRINTF("Catch Exception %s ... %s\n", __FUNCTION__, e.what());
            return EC_BOARD_NOK;
        }

        return EC_BOARD_OK;

    }

    virtual int start(int controller_type) {
        
        // pid not used
        return start(controller_type, 0, 0, 0);        
    }

    virtual int stop(void) {
        return set_ctrl_status_X(this, CTRL_POWER_MOD_OFF);
    }

    virtual void start_log(bool start) {
        Log::start_log(start);
    }

    virtual void handle_fault(void) {

        fault_t fault;
        fault.all = rx_pdo.fault;
        //fault.bit.
        tx_pdo.fault_ack = fault.all & 0xFFFF;
        //ack_faults_X(this, fault.all);

    }

        /////////////////////////////////////////////
    // set pdo data
    virtual int set_posRef(float joint_pos) { tx_pdo.pos_ref = joint_pos; }
    virtual int set_torOffs(float tor_offs) { /*tx_pdo.tor_offs = tor_offs;*/ }
    virtual int set_posGainP(float p_gain)  { /*tx_pdo.PosGainP = p_gain;*/   }
    virtual int set_posGainI(float i_gain)  { /*tx_pdo.PosGainI = i_gain;*/   }
    virtual int set_posGainD(float d_gain)  { /*tx_pdo.PosGainD = d_gain;*/   }

    virtual int move_to(float pos_ref, float step) {
    
        float pos, tx_pos_ref;
        
        try {
            readSDO_byname("link_pos", pos);
            readSDO_byname("pos_ref_fb", tx_pos_ref);
            //tx_pos_ref = pos_ref;
            if ( fabs(pos - pos_ref) > step ) {
                if ( pos > pos_ref ) {
                    tx_pos_ref -= step*2; 
                } else {
                    tx_pos_ref += step*2;
                }
                    
                writeSDO_byname("pos_ref", tx_pos_ref);
                DPRINTF("%d move to %f %f %f\n", Joint_robot_id, pos_ref, tx_pos_ref, pos);
                return 0;
            } else {
                DPRINTF("%d move to %f %f %f\n", Joint_robot_id, pos_ref, tx_pos_ref, pos);
                return 1;
            }
            
        } catch (EscWrpError &e ) {
            DPRINTF("Catch Exception %s ... %s\n", __FUNCTION__, e.what());
            return 0;
        }
    
    }

    virtual void set_off_sgn(float offset, int sgn) {}

private:

    int16_t Joint_robot_id;
    
    float   _offset;
    int     _sgn;

    stat_t  s_rtt;

    objd_t * SDOs;

};



}
}
}
#endif /* IIT_ECAT_ADVR_ESC_H_ */

/*
 * ft6_esc.h
 * 
 *  Force Toruqe sensor
 *  based on TI tm4c123AH6PM - Tiva Microcontroller
 *  
 *  http://www.ti.com/product/tm4c123ah6pm
 *  
 *  Created on: Jan 2015
 *      Author: alessio margan
 */

#ifndef __IIT_ECAT_ADVR_FT6_ESC_H__
#define __IIT_ECAT_ADVR_FT6_ESC_H__

#include <iit/ecat/slave_wrapper.h>
#include <iit/ecat/advr/esc.h>
#include <iit/ecat/advr/log_esc.h>
#include <iit/ecat/utils.h>
#include <map>

namespace iit {
    namespace ecat {
        namespace advr {


  struct Ft6EscPdoTypes {
    // TX  slave_input -- master output
    struct pdo_tx {
        uint64_t    ts;
    }  __attribute__((__packed__));

    // RX  slave_output -- master input
    struct pdo_rx {
        float       force_X;            // N
        float       force_Y;            // N
        float       force_Z;            // N
        float       torque_X;           // Nm
        float       torque_Y;           // Nm
        float       torque_Z;           // Nm
        uint16_t    fault;
        uint64_t    rtt;                // ns
        void sprint(char *buff, size_t size) {
            snprintf(buff, size, "%f\t%f\t%f\t%f\t%f\t%f\t%d\t%lu\n", force_X,force_Y,force_Z,torque_X,torque_Y,torque_Z,fault,rtt);
        }
        void fprint(FILE *fp) {
            fprintf(fp, "%f\t%f\t%f\t%f\t%f\t%f\t%d\t%lu\n", force_X,force_Y,force_Z,torque_X,torque_Y,torque_Z,fault,rtt);
        }
    }  __attribute__((__packed__));
};


struct Ft6EscSdoTypes {

    // flash

    unsigned long Block_control;
    long NumAvSamples;   

    unsigned long calibration_offset0;
    unsigned long calibration_offset1;
    unsigned long calibration_offset2;
    unsigned long calibration_offset3;
    unsigned long calibration_offset4;
    unsigned long calibration_offset5;

    float matrix_r1_c1;
    float matrix_r1_c2;
    float matrix_r1_c3;
    float matrix_r1_c4;
    float matrix_r1_c5;
    float matrix_r1_c6;

    int16_t sensor_number;
    int16_t sensor_robot_id;

    // ram

    char        firmware_version[8];
    uint16_t    ack_board_fault;
    float       matrix_rn_c1;
    float       matrix_rn_c2;
    float       matrix_rn_c3;
    float       matrix_rn_c4;
    float       matrix_rn_c5;
    float       matrix_rn_c6;
    uint16_t    flash_params_cmd;
    uint16_t    flash_params_cmd_ack;     
};

/**
*  
**/ 

class Ft6ESC :
    public BasicEscWrapper<Ft6EscPdoTypes, Ft6EscSdoTypes>, 
    public PDO_log<Ft6EscPdoTypes::pdo_rx>,
    public XDDP_pipe<Ft6EscPdoTypes::pdo_rx, Ft6EscPdoTypes::pdo_tx>
{
public:
    typedef BasicEscWrapper<Ft6EscPdoTypes,Ft6EscSdoTypes>              Base;
    typedef PDO_log<Ft6EscPdoTypes::pdo_rx>                             Log;
    typedef XDDP_pipe<Ft6EscPdoTypes::pdo_rx,Ft6EscPdoTypes::pdo_tx>    Xddp;

public:
    Ft6ESC(const ec_slavet& slave_descriptor) :
        Base(slave_descriptor),
        Log(std::string("/tmp/Ft6ESC_pos"+std::to_string(position)+"_log.txt"),DEFAULT_LOG_SIZE),
        Xddp()
    { }

    virtual ~Ft6ESC(void) {
        delete [] SDOs;
        DPRINTF("~%s pos %d\n", typeid(this).name(), position);
        print_stat(s_rtt);
    }
    
    int set_cal_matrix(std::vector<std::vector<float>> &cal_matrix);

    virtual const objd_t * get_SDOs() { return SDOs; }
    virtual void init_SDOs(void);
    virtual uint16_t get_ESC_type() { return FT6; }

    virtual void on_writePDO(void) {

        tx_pdo.ts = get_time_ns();
    }

    virtual void on_readPDO(void) {

        if ( rx_pdo.rtt ) {
            rx_pdo.rtt =  get_time_ns() - rx_pdo.rtt;
            s_rtt(rx_pdo.rtt);
        }

        if ( rx_pdo.fault & 0xFFFF) {
            //handle_fault();
        }

        if ( _start_log ) {
            Log::log_t log;
            //log.ts = get_time_ns() -_start_log_ts ;
            log.force_X     = rx_pdo.force_X;
            log.force_Y     = rx_pdo.force_Y;
            log.force_Z     = rx_pdo.force_Z;
            log.torque_X    = rx_pdo.torque_X;
            log.torque_Y    = rx_pdo.torque_Y;
            log.torque_Z    = rx_pdo.torque_Z;  
            log.fault       = rx_pdo.fault;     
            log.rtt         = rx_pdo.rtt;       
            push_back(log);
        }

        Xddp::xddp_tx_t xddp_tx;
        xddp_tx.force_X     = rx_pdo.force_X; 
        xddp_tx.force_Y     = rx_pdo.force_Y; 
        xddp_tx.force_Z     = rx_pdo.force_Z; 
        xddp_tx.torque_X    = rx_pdo.torque_X;
        xddp_tx.torque_Y    = rx_pdo.torque_Y;
        xddp_tx.torque_Z    = rx_pdo.torque_Z;
        xddp_tx.fault       = rx_pdo.fault;   
        xddp_tx.rtt         = rx_pdo.rtt;     
        xddp_write(xddp_tx);
    }

    int16_t get_robot_id() {
        //assert(sdo.Joint_robot_id != -1);
        return sdo.sensor_robot_id;
    }

    virtual int init(const YAML::Node & root_cfg) {

        int16_t robot_id = -1;

        try {
            init_SDOs();
            init_sdo_lookup();
            getSDO_byname("Sensor_robot_id", robot_id);
            //set_flash_cmd_X(this, CTRL_REMOVE_TORQUE_OFFS);

        } catch (EscWrpError &e ) {

            DPRINTF("Catch Exception in %s ... %s\n", __PRETTY_FUNCTION__, e.what());
            return EC_BOARD_INIT_SDO_FAIL;
        }

#if 0
        if ( robot_id > 0 ) {
            try {
                std::string esc_conf_key = std::string("Ft6ESC_"+std::to_string(robot_id));
                const YAML::Node& esc_conf = root_cfg[esc_conf_key];
                if ( esc_conf.Type() != YAML::NodeType::Null ) {
                }
            } catch (YAML::KeyNotFound &e) {
                DPRINTF("Catch Exception in %s ... %s\n", __PRETTY_FUNCTION__, e.what());
                return EC_BOARD_KEY_NOT_FOUND;
            }
        }
#endif
        // set filename with robot_id
        log_filename = std::string("/tmp/Ft6ESC_"+std::to_string(sdo.sensor_robot_id)+"_log.txt");
        Xddp::init(std::string("Ft6ESC_"+std::to_string(sdo.sensor_robot_id)));
    
        return EC_BOARD_OK;

    }
    
    virtual void start_log(bool start) {
        Log::start_log(start);
    }

    virtual void handle_fault(void) {

        fault_t fault;
        fault.all = rx_pdo.fault;
        //fault.bit.
        ack_faults_X(this, fault.all);

    }

private:
    stat_t  s_rtt;
    objd_t * SDOs;
};


typedef std::map<int, Ft6ESC*>  FtSlavesMap;

inline int Ft6ESC::set_cal_matrix(std::vector<std::vector<float>> &cal_matrix)
{
    int     res = 0;
    int16_t ack;
    int16_t flash_row_cmd = 0x00C7;
    
    for ( int r=0; r<6; r++ ) {
    
        // set row n param value
        writeSDO_byname("matrix_rn_c1",cal_matrix[r][0]);
        writeSDO_byname("matrix_rn_c2",cal_matrix[r][1]);
        writeSDO_byname("matrix_rn_c3",cal_matrix[r][2]);
        writeSDO_byname("matrix_rn_c4",cal_matrix[r][3]);
        writeSDO_byname("matrix_rn_c5",cal_matrix[r][4]);
        writeSDO_byname("matrix_rn_c6",cal_matrix[r][5]);
    
        if ( set_flash_cmd_X(this, flash_row_cmd) != EC_BOARD_OK ) {
            return EC_BOARD_FT6_CALIB_FAIL;
        }
        // next row cmd
        flash_row_cmd++;

    } // for rows

    return EC_BOARD_OK;
}



}
}
}
#endif /* __IIT_ECAT_ADVR_FT6_ESC_H__ */
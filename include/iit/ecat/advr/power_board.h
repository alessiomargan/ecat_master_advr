/*
 *
 *  Created on: May, 2015
 *      Author: alessio margan
 */

#ifndef __IIT_ECAT_ADVR_POW_ESC_H__
#define __IIT_ECAT_ADVR_POW_ESC_H__

#include <iit/ecat/advr/esc.h>
#include <iit/ecat/advr/log_esc.h>
#include <iit/ecat/advr/pipes.h>
#include <protobuf/ecat_pdo.pb.h>

#include <map>
#include <string>


namespace iit {
namespace ecat {
namespace advr {

struct status_bits {
    uint16_t  key_status:1;
    uint16_t  vsc_status:1;
    uint16_t  cpu_rel_status:1;
    uint16_t  prech_rel_status:1;
    uint16_t  main_rel_status:1;
    uint16_t  fan_1_status:1;
    uint16_t  fan_2_status:1;
    uint16_t  spare:1;
    uint16_t  state_machine_status:8;
};

typedef union {
    uint16_t all;
    struct status_bits bit;
} status_t;


struct PowEscPdoTypes {
    // TX  slave_input -- master output
    struct pdo_tx {
        uint16_t    master_command;
        uint16_t    fault_ack;
        uint16_t    ts;
    } __attribute__ ( ( __packed__ ) );

    // RX  slave_output -- master input
    struct pdo_rx {
        status_t    status;
        uint16_t    board_temp;         // °C
        uint16_t    battery_temp;       // °C
        uint16_t    battery_volt;       // V
        int16_t     battery_curr;       // A
        int16_t     load_curr;          // A
        uint16_t    fault;
        uint16_t    rtt;                // us
        int sprint ( char *buff, size_t size ) {
            return snprintf ( buff, size, "0x%02X\t%d\t%d\t%d", status.all,board_temp,board_temp,rtt );
        }
        void fprint ( FILE *fp ) {
            fprintf ( fp, "0x%02X\t%d\t%d\t%d", status.all,board_temp,board_temp,rtt );
        }
        void to_map ( jmap_t & jpdo ) {
            JPDO ( status.all );
            JPDO ( rtt );
        }
        void pb_toString( std::string * pb_str ) {
            static iit::advr::Ec_slave_pdo pb_rx_pdo;
            static struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            // Header
            pb_rx_pdo.mutable_header()->mutable_stamp()->set_sec(ts.tv_sec);
            pb_rx_pdo.mutable_header()->mutable_stamp()->set_nsec(ts.tv_nsec);
            // Type
            pb_rx_pdo.set_type(iit::advr::Ec_slave_pdo::RX_POW_WLK);
            // PowWalkman_tx_pdo
            pb_rx_pdo.mutable_powwalkman_rx_pdo()->set_status(status.all);
            pb_rx_pdo.mutable_powwalkman_rx_pdo()->set_temperature(board_temp);
            pb_rx_pdo.mutable_powwalkman_rx_pdo()->set_batt_temp(battery_temp);
            pb_rx_pdo.mutable_powwalkman_rx_pdo()->set_batt_volt(battery_volt);
            pb_rx_pdo.mutable_powwalkman_rx_pdo()->set_batt_curr(battery_curr);
            pb_rx_pdo.mutable_powwalkman_rx_pdo()->set_load_curr(load_curr);
            pb_rx_pdo.mutable_powwalkman_rx_pdo()->set_fault(fault);
            pb_rx_pdo.mutable_powwalkman_rx_pdo()->set_rtt(rtt);
            pb_rx_pdo.SerializeToString(pb_str);
        }
    } __attribute__ ( ( __packed__ ) );
};


struct PowEscSdoTypes {
    // flash param
    //...
    // ram param
    char        firmware_version[8];
    uint16_t    ctrl_status_cmd;
    uint16_t    ctrl_status_cmd_ack;
    uint16_t    flash_params_cmd;
    uint16_t    flash_params_cmd_ack;
    uint16_t    v_pack_adc;
    uint16_t    v_batt_adc;
    uint16_t    i_batt_adc;
    uint16_t    i_load_adc;
    uint16_t    t_batt_adc;
    uint16_t    t_board_adc;
    uint16_t    FSM;
    float       v_batt_filt;
    float       v_pack_filt;
};

class PowESC :
    public BasicEscWrapper<PowEscPdoTypes,PowEscSdoTypes>,
    public PDO_log<PowEscPdoTypes::pdo_rx>,
    public XDDP_pipe
{
public:
    typedef BasicEscWrapper<PowEscPdoTypes,PowEscSdoTypes>    Base;
    typedef PDO_log<PowEscPdoTypes::pdo_rx>                    Log;

    PowESC ( const ec_slavet& slave_descriptor ) :
        Base ( slave_descriptor ),
        Log ( std::string ( "/tmp/PowESC_pos"+std::to_string ( position ) +"_log.txt" ),DEFAULT_LOG_SIZE ),
        XDDP_pipe()
    {
        _start_log = false;
    }

    virtual ~PowESC ( void ) {
        delete [] SDOs;
        DPRINTF ( "~%s %d\n", typeid ( this ).name(), position );
        print_stat ( s_rtt );
    }

    virtual void on_readPDO ( void ) {

        if ( rx_pdo.rtt ) {
            rx_pdo.rtt = ( uint16_t ) ( get_time_ns() /1000 - rx_pdo.rtt );
            s_rtt ( rx_pdo.rtt );
        }

        handle_status();

        if ( _start_log ) {
            push_back ( rx_pdo );
        }

//         xddp_write ( rx_pdo );
    }

    virtual void on_writePDO ( void ) {
        tx_pdo.ts = ( uint16_t ) ( get_time_ns() /1000 );
    }

    virtual const objd_t * get_SDOs() {
        return SDOs;
    }
    virtual uint32_t get_ESC_type() {
        return POW_BOARD;
    }

    void print_info ( void ) {
        DPRINTF ( "\tfw_ver %s\n", std::string((const char *)sdo.firmware_version,8).c_str() );
    }

    void init_SDOs ( void );

    int init ( const YAML::Node & root_cfg ) {

        try {
            init_SDOs();
            init_sdo_lookup();

        } catch ( EscWrpError &e ) {

            DPRINTF ( "Catch Exception in %s ... %s\n", __PRETTY_FUNCTION__, e.what() );
            return EC_BOARD_INIT_SDO_FAIL;
        }

        // we log when receive PDOs
        start_log ( true );

//         XDDP_pipe::init( "PowWkm_pos_"+std::to_string ( position ) );
        
        osal_timer_start ( &motor_on_timer, 0 );
        readSDO_byname ( "status" );
        handle_status();

        set_ctrl_status_X ( this, CTRL_FAN_1_ON );
        set_ctrl_status_X ( this, CTRL_FAN_2_ON );

        return EC_BOARD_OK;
    }

    void handle_status ( void ) {

        static status_t status;

        if ( rx_pdo.status.bit.key_status != status.bit.key_status ) {
            // parking and shutdown
            if ( rx_pdo.status.bit.key_status ) {
                DPRINTF ( "KEY ON\n" );
            } else {
                DPRINTF ( "KEY OFF\n" );
            }
        }

        if ( rx_pdo.status.bit.vsc_status != status.bit.vsc_status ) {
            // red button
            if ( rx_pdo.status.bit.vsc_status ) {
                DPRINTF ( "VCS ON press \n" );
            } else {
                DPRINTF ( "VCS OFF release\n" );
            }
        }

        if ( osal_timer_is_expired ( &motor_on_timer ) ) {
            if ( rx_pdo.status.bit.state_machine_status == RUN_WAIT_POWER_MOTORS_COMMAND_FSM ) {
                //
                set_ctrl_status_X ( this, CTRL_POWER_MOTORS_ON );
                // set 5 sec to check again
                osal_timer_start ( &motor_on_timer, 5000000 );
            }
        }

        status.all = rx_pdo.status.all;
    }

    int power_on_ok ( void ) {

        return rx_pdo.status.bit.main_rel_status == 1;
    }

private:

    osal_timer  motor_on_timer;

    YAML::Node node_cfg;

    objd_t * SDOs;

    stat_t  s_rtt;

};



}
}
}

#endif


// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

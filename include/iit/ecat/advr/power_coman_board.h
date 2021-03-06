/*
 *
 *  Created on: May, 2015
 *      Author: alessio margan
 */

#ifndef __IIT_ECAT_ADVR_POW_COMAN_ESC_H__
#define __IIT_ECAT_ADVR_POW_COMAN_ESC_H__

#include <iit/ecat/advr/esc.h>
#include <iit/ecat/advr/log_esc.h>
#include <iit/ecat/advr/pipes.h>

#include <map>
#include <string>


namespace iit {
namespace ecat {
namespace advr {

#define CTRL_POWER_MOTORS_ON    0x0048
#define CTRL_POWER_MOTORS_OFF   0x0084
#define CTRL_POWER_ROBOT_OFF    0x00DD


// Run State machine states definition
#define RUN_INIT_FSM                        0
#define RUN_POWER_CPU_RELAY                 1
#define RUN_POWER_MOTOR_RELAYS_FSM          2
#define RUN_WAIT_POWER_MOTORS_COMMAND_FSM   3
#define RUN_LOOP_FSM                        4
#define RUN_POWER_ROBOT_OFF_FSM             5

struct status_cmn_pow_bits {
    uint8_t main_rel_status:1;
    uint8_t pwr_sw_status:1;
    uint8_t prech_rel_status:1;
    uint8_t spare_bit_4:1;
    uint8_t spare_bit_5:1;
    uint8_t spare_bit_6:1;
    uint8_t spare_bit_7:1;
    uint8_t spare_bit_8:1;
    uint8_t status_fsm;
};

typedef union {
    uint16_t all;
    struct status_cmn_pow_bits bit;
} status_cmn_pow_t;


struct PowCmnEscPdoTypes {
    
    // TX  slave_input -- master output
    struct pdo_tx {
        uint16_t    master_command;
        uint16_t    ts;
        
        std::ostream& dump ( std::ostream& os, const std::string delim ) const {
            os << master_command << delim;
            os << ts << delim;
            //os << std::endl;
            return os;
        }
        
    } __attribute__ ( ( __packed__ ) );

    // RX  slave_output -- master input
    struct pdo_rx {
        float               temperature;
        float               v_batt;
        status_cmn_pow_t    status;
        uint16_t            rtt;
        
        std::ostream& dump ( std::ostream& os, const std::string delim ) const {
            os << temperature << delim;
            os << v_batt << delim;
            os << status.all << delim;
            os << rtt << delim;
            //os << std::endl;
            return os;
        }
        void fprint ( FILE *fp ) {
            std::ostringstream oss;
            dump(oss,"\t");
            fprintf ( fp, "%s", oss.str().c_str() );
        }
        int sprint ( char *buff, size_t size ) {
            std::ostringstream oss;
            dump(oss,"\t");
            return snprintf ( buff, size, "%s", oss.str().c_str() );
        }
        void to_map ( jmap_t & jpdo ) {
            JPDO ( temperature );
            JPDO ( v_batt );
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
            pb_rx_pdo.set_type(iit::advr::Ec_slave_pdo::RX_POW_CMN);
            // PowWalkman_rx_pdo
            pb_rx_pdo.mutable_powcoman_rx_pdo()->set_temperature(temperature);
            pb_rx_pdo.mutable_powcoman_rx_pdo()->set_v_batt(v_batt);
            pb_rx_pdo.mutable_powcoman_rx_pdo()->set_status(status.all);
            pb_rx_pdo.mutable_powcoman_rx_pdo()->set_rtt(rtt);
            pb_rx_pdo.SerializeToString(pb_str);
        }
        
    } __attribute__ ( ( __packed__ ) );
};

inline std::ostream& operator<< (std::ostream& os, const PowCmnEscPdoTypes::pdo_tx& tx_pdo ) {
    return tx_pdo.dump(os,"\t");
}

inline std::ostream& operator<< (std::ostream& os, const PowCmnEscPdoTypes::pdo_rx& rx_pdo ) {
    return rx_pdo.dump(os,"\t");
}


struct PowCmnEscSdoTypes {
    // flash param
    //...
    // ram param
    char        firmware_version[8];
    float       temperature_filtered;
    float       v_batt_filtered;
    float       v_pack_filtered;
    uint16_t    ctrl_status_cmd;
    uint16_t    ctrl_status_cmd_ack;
};


class PowComanESC :
    public BasicEscWrapper<PowCmnEscPdoTypes,PowCmnEscSdoTypes>,
    public PDO_log<PowCmnEscPdoTypes::pdo_rx>,
    public XDDP_pipe
{
public:
    typedef BasicEscWrapper<PowCmnEscPdoTypes,PowCmnEscSdoTypes>	Base;
    typedef PDO_log<PowCmnEscPdoTypes::pdo_rx>                    	Log;

    PowComanESC ( const ec_slavet& slave_descriptor ) :
        Base ( slave_descriptor ),
        Log ( std::string ( "/tmp/PowCmnESC_pos"+std::to_string ( position ) +"_log.txt" ),DEFAULT_LOG_SIZE ) {
        _start_log = false;
        //_actual_state = EC_STATE_PRE_OP;
    }

    virtual ~PowComanESC ( void ) {
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

        xddp_write ( rx_pdo );
    }

    virtual void on_writePDO ( void ) {
        tx_pdo.ts = ( uint16_t ) ( get_time_ns() /1000 );
    }

    virtual const objd_t * get_SDOs() 	{
        return SDOs;
    }
    virtual uint32_t get_ESC_type ( void ) {
        return POW_CMN_BOARD;
    }

    void print_info ( void ) { return; }

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

        XDDP_pipe::init( "PowCmn_pos_"+std::to_string ( position ));
        
        osal_timer_start ( &motor_on_timer, 0 );
        readSDO_byname ( "status" );
        handle_status();

        return EC_BOARD_OK;
    }

    void handle_status ( void ) {

        static status_cmn_pow_t status;
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

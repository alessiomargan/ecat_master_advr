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

#include <map>

#include <iit/ecat/slave_wrapper.h>
#include <iit/ecat/advr/esc.h>
#include <iit/ecat/advr/log_esc.h>
#include <iit/ecat/advr/pipes.h>
#include <iit/ecat/utils.h>

namespace iit {
namespace ecat {
namespace advr {


struct Ft6EscPdoTypes {
    
    // TX  slave_input -- master output
    struct pdo_tx {
        uint16_t    ts;
        
        std::ostream& dump ( std::ostream& os, const std::string delim ) const {
            os << ts << delim;
            //os << std::endl;
            return os;
        }

    }  __attribute__ ( ( __packed__ ) );

    // RX  slave_output -- master input
    struct pdo_rx {
        float       force_X;            // N
        float       force_Y;            // N
        float       force_Z;            // N
        float       torque_X;           // Nm
        float       torque_Y;           // Nm
        float       torque_Z;           // Nm
        uint16_t    fault;
        uint16_t    rtt;                // ns
        
        std::ostream& dump ( std::ostream& os, const std::string delim ) const {
            os << force_X << delim;
            os << force_Y << delim;
            os << force_Z << delim;
            os << torque_X << delim;
            os << torque_Y << delim;
            os << torque_Z << delim;
            os << fault << delim;
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
            JPDO ( force_X );
            JPDO ( force_Y );
            JPDO ( force_Z );
            JPDO ( torque_X );
            JPDO ( torque_Y );
            JPDO ( torque_Z );
            JPDO ( fault );
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
            pb_rx_pdo.set_type(iit::advr::Ec_slave_pdo::RX_FT6);
            // FT6_rx_pdo
            pb_rx_pdo.mutable_ft6_rx_pdo()->set_force_x(force_X);
            pb_rx_pdo.mutable_ft6_rx_pdo()->set_force_y(force_Y);
            pb_rx_pdo.mutable_ft6_rx_pdo()->set_force_z(force_Z);
            pb_rx_pdo.mutable_ft6_rx_pdo()->set_torque_x(torque_X);
            pb_rx_pdo.mutable_ft6_rx_pdo()->set_torque_y(torque_Y);
            pb_rx_pdo.mutable_ft6_rx_pdo()->set_torque_z(torque_Z);
            pb_rx_pdo.mutable_ft6_rx_pdo()->set_fault(fault);
            pb_rx_pdo.mutable_ft6_rx_pdo()->set_rtt(rtt);
            pb_rx_pdo.SerializeToString(pb_str);
        }

    }  __attribute__ ( ( __packed__ ) );
};

inline std::ostream& operator<< (std::ostream& os, const Ft6EscPdoTypes::pdo_tx& tx_pdo ) {
    return tx_pdo.dump(os,"\t");
}

inline std::ostream& operator<< (std::ostream& os, const Ft6EscPdoTypes::pdo_rx& rx_pdo ) {
    return rx_pdo.dump(os,"\t");
}

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

struct Ft6LogTypes {

    uint64_t                ts;     // ns
    Ft6EscPdoTypes::pdo_rx  rx_pdo;

    void fprint ( FILE *fp ) {
        fprintf ( fp, "%lu\t", ts );
        rx_pdo.fprint ( fp );
    }
    int sprint ( char *buff, size_t size ) {
        int l = snprintf ( buff, size, "%lu\t", ts );
        return l + rx_pdo.sprint ( buff+l,size-l );
    }
};


/**
*
**/

class Ft6ESC :
    public BasicEscWrapper<Ft6EscPdoTypes, Ft6EscSdoTypes>,
    public PDO_log<Ft6LogTypes>,
    public XDDP_pipe
{
public:
    typedef BasicEscWrapper<Ft6EscPdoTypes,Ft6EscSdoTypes>   Base;
    typedef PDO_log<Ft6LogTypes>                             Log;

public:
    Ft6ESC ( const ec_slavet& slave_descriptor ) :
        Base ( slave_descriptor ),
        Log ( std::string ( "/tmp/Ft6ESC_pos"+std::to_string ( position ) +"_log.txt" ),DEFAULT_LOG_SIZE ),
        XDDP_pipe ()
    { 
        _start_log = false;
    }

    virtual ~Ft6ESC ( void ) {
        delete [] SDOs;
        DPRINTF ( "~%s pos %d\n", typeid ( this ).name(), position );
        print_stat ( s_rtt );
    }

    int set_cal_matrix ( std::vector<std::vector<float>> &cal_matrix );

    virtual const objd_t * get_SDOs() {
        return SDOs;
    }
    virtual void init_SDOs ( void );
    virtual uint32_t get_ESC_type() {
        return FT6;
    }

    virtual void on_writePDO ( void ) {

        tx_pdo.ts = ( uint16_t ) ( get_time_ns() /1000 );
    }

    virtual void on_readPDO ( void ) {

        if ( rx_pdo.rtt ) {
            rx_pdo.rtt = ( uint16_t ) ( get_time_ns() /1000 ) - rx_pdo.rtt;
            s_rtt ( rx_pdo.rtt );
        }

        if ( rx_pdo.fault ) {
            handle_fault();
        } else {
            // clean any previuos fault ack !!
            //tx_pdo.fault_ack = 0;
        }

        if ( _start_log ) {
            Log::log_t log;
            log.ts      = get_time_ns() - _start_log_ts ;
            log.rx_pdo  = rx_pdo;
            push_back ( log );
        }
        
        if( use_pipes ) {
            xddp_write ( rx_pdo );
        }

    }

    int16_t get_robot_id() {
        //assert(sdo.Joint_robot_id != -1);
        return sdo.sensor_robot_id;
    }

    void print_info ( void ) {
        DPRINTF ( "\tSensor id %d\tSensor robot id %d\n", sdo.sensor_number, sdo.sensor_robot_id );
        DPRINTF ( "\tfw_ver %s\n", sdo.firmware_version );
    }

    virtual int init ( const YAML::Node & root_cfg ) {

        std::string robot_name("void");
        try {
            robot_name = root_cfg["ec_boards_base"]["robot_name"].as<std::string>();
        } catch ( YAML::Exception &e ) {
        }

        try {
            init_SDOs();
            init_sdo_lookup();
            readSDO_byname ( "Sensor_robot_id" );
            set_flash_cmd_X ( this, CTRL_REMOVE_TORQUE_OFFS );

        } catch ( EscWrpError &e ) {

            DPRINTF ( "Catch Exception in %s ... %s\n", __PRETTY_FUNCTION__, e.what() );
            return EC_BOARD_INIT_SDO_FAIL;
        }

#if 0
        if ( robot_id > 0 ) {
            try {
                std::string esc_conf_key = std::string ( "Ft6ESC_"+std::to_string ( robot_id ) );
                const YAML::Node& esc_conf = root_cfg[esc_conf_key];
                if ( esc_conf.Type() != YAML::NodeType::Null ) {
                }
            } catch ( YAML::KeyNotFound &e ) {
                DPRINTF ( "Catch Exception in %s ... %s\n", __PRETTY_FUNCTION__, e.what() );
                return EC_BOARD_KEY_NOT_FOUND;
            }
        }
#endif
        // set filename with robot_id
        log_filename = std::string ( "/tmp/Ft6ESC_"+std::to_string ( sdo.sensor_robot_id ) +"_log.txt" );

        // we log when receive PDOs
        start_log ( true );

        // set use pipe variable NOTE true by default
        if(root_cfg["ec_board_ctrl"]["use_pipes"]) {
            use_pipes = root_cfg["ec_board_ctrl"]["use_pipes"].as<bool>();
        }
        
        if( use_pipes ) {
            XDDP_pipe::init( robot_name+"@Ft_id_"+std::to_string ( get_robot_id() ) );
        }
        
        return EC_BOARD_OK;

    }

    virtual void start_log ( bool start ) {
        Log::start_log ( start );
    }

    virtual void handle_fault ( void ) {

        fault_t fault;
        fault.all = rx_pdo.fault;
        //fault.bit.
        //ack_faults_X(this, fault.all);

    }

private:
    stat_t  s_rtt;
    objd_t * SDOs;
};


typedef std::map<int, Ft6ESC*>  FtSlavesMap;

inline int Ft6ESC::set_cal_matrix ( std::vector<std::vector<float>> &cal_matrix ) {
    int     res = 0;
    int16_t ack;
    int16_t flash_row_cmd = 0x00C7;

    for ( int r=0; r<6; r++ ) {

        // set row n param value
        writeSDO_byname ( "matrix_rn_c1",cal_matrix[r][0] );
        writeSDO_byname ( "matrix_rn_c2",cal_matrix[r][1] );
        writeSDO_byname ( "matrix_rn_c3",cal_matrix[r][2] );
        writeSDO_byname ( "matrix_rn_c4",cal_matrix[r][3] );
        writeSDO_byname ( "matrix_rn_c5",cal_matrix[r][4] );
        writeSDO_byname ( "matrix_rn_c6",cal_matrix[r][5] );

        if ( set_flash_cmd_X ( this, flash_row_cmd ) != EC_BOARD_OK ) {
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
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

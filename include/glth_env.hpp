#ifndef __glth_env_hpp
#define __glth_env_hpp

#include <string>
#include <sstream>
#include <stdint.h>

#include "glth_const.hpp"


namespace glth
{

    // configuration bitmasks
    typedef uint64_t bitmask;
    static bitmask _name_set = (2 << 0);

    class env
    {
        private:
            // Default constructor is private because why do you need that?
            env();

            std::string _in_filename;
            std::string _out_filename;
            size_t _freq_res;
            size_t _time_res;
            size_t _time_step;

            uint64_t _conf_mask;

            template< class arg_t >
            void bind( const char* arg, void (env::*setter)( arg_t ) );


        public:
            // Factory method from arguments
            static env const* env_from_args( int argc, char** argv );

            // Verify that an environment is sensible e.g. that required fields
            // have been set.
            static glth_const::env_result verify( const glth::env );

            // getters/setters
            void set_in_filename( std::string );
            std::string get_in_filename() const;

            void set_out_filename( std::string );
            std::string get_out_filename() const;

            void set_freq_res( size_t );
            size_t get_freq_res() const;

            void set_time_res( size_t );
            size_t get_time_res() const;

            void set_time_step( size_t );
            size_t get_time_step() const;
    };

}
;
// namespace glth

#endif // __glth_env_hpp

#include "glth_env.hpp"

glth::env::env() :
    _conf_mask( 0 )
{ /* no-op */
}

//can get away with having definition of template function
//in cpp file because it's private and will only be instantiated
//from this file.

template< class arg_t >
inline void glth::env::bind( const char* arg, void (env::*setter)( arg_t ) )
{
    arg_t value;
    std::stringstream converter;
    converter << arg;
    converter >> value;
    (this->*setter)( value );
    return;
}

glth::env const* glth::env::env_from_args( int argc, char** argv )
{
    glth::env* res = new glth::env();
    std::stringstream converter;

    int c;
    while( (c = getopt( argc, argv, "i:F:T:t:" )) != -1 )
    {
        converter.str("");
        converter.clear();
        switch( c )
        {
            case 'i':
                res->set_in_filename( optarg );
                (res->_conf_mask) |= _name_set;
                break;
            case 'F':
                res->bind( optarg, &env::set_freq_res );
                break;
            case 'T':
                res->bind( optarg, &env::set_time_res );
                break;
            case 't':
                res->bind( optarg, &env::set_time_step );
                break;
        }
    }

    return res;
}

void glth::env::set_in_filename( std::string new_filename )
{
    this->_in_filename = new_filename;
}
std::string glth::env::get_in_filename() const
{
    return (this->_in_filename);
}

void glth::env::set_out_filename( std::string new_filename )
{
    this->_out_filename = new_filename;
}
std::string glth::env::get_out_filename() const
{
    return (this->_out_filename);
}

void glth::env::set_freq_res( size_t value )
{
    this->_freq_res = value;
}
size_t glth::env::get_freq_res() const
{
    return (this->_freq_res);
}

void glth::env::set_time_res( size_t value )
{
    this->_time_res = value;
}
size_t glth::env::get_time_res() const
{
    return (this->_time_res);
}

void glth::env::set_time_step( size_t value )
{
    this->_time_step = value;
}
size_t glth::env::get_time_step() const
{
    return (this->_time_step);
}

glth_const::env_result glth::env::verify( const glth::env tgt )
{
    glth_const::env_result res = glth_const::env_success;

    // Check that the filename is set.
    if( (tgt._conf_mask & _name_set) != _name_set )
    {
        res = glth_const::env_no_filename;
    }

    return res;
}

#include "glth_io.hpp"

glth::io::io() :
    _monarch_ptr( NULL )
{/* no-op */
}

glth::io* glth::io::open_file( std::string filename )
{
    glth::io* _io_ptr = new glth::io();

    _io_ptr->_monarch_ptr = Monarch::OpenForReading( filename );

    // If the monarch pointer is not NULL, we're good.
    if( _io_ptr->_monarch_ptr == NULL )
    {
        delete _io_ptr;
        _io_ptr = NULL;
    }

    else if( _io_ptr->_monarch_ptr->ReadHeader() == false )
    {
        delete _io_ptr;
        _io_ptr = NULL;
    }

    return _io_ptr;
}

std::size_t glth::io::get_record_length() const
{
    return (this->_monarch_ptr->GetHeader()->GetRecordSize());
}
double glth::io::get_record_frequency() const
{
    return (this->_monarch_ptr->GetHeader()->GetAcqRate());
}

glth_const::io_result glth::io::populate( signal& ch1, signal& ch2 )
{
    static const MonarchRecord* work_rec;

    // The output arrays need to be of the correct shape.
    if( (ch1.size()) != (this->get_record_length()) || (ch2.size()) != (this->get_record_length()) )
    {
        return glth_const::io_read_badarray;
    }

    // OK
    else
    {
        // Try to grab the next record.  It might be EOF, for example, but
        // give it a shot.
        if( this->_monarch_ptr->ReadRecord() == true )
        {

            // Grab the first channel of data.
            work_rec = (this->_monarch_ptr->GetRecordOne());

            // It had better be channel 1...
            if( work_rec->fCId != 1 )
            {
                // fuck
                return glth_const::io_channel_err;
            }

            // D2A the data from channel 1.
            for( std::size_t idx = 0; idx < ch1.size(); idx++ )
            {
                double pt = 0;
                ch1[idx][0] = glth_px1500::min_mvolts + (glth_px1500::max_mvolts) * (work_rec->fDataPtr[idx]) / (2 << glth_px1500::n_bits);
                ch1[idx][1] = 0.;
            }

            // Grab the next channel of data.
            work_rec = (this->_monarch_ptr->GetRecordTwo());

            // It had better be channel 2...
            if( work_rec->fCId != 2 )
            {
                // fuck
                return glth_const::io_channel_err;
            }

            // And if it is, we D2A it again.
            for( std::size_t idx = 0; idx < ch2.size(); idx++ )
            {
                double pt = 0;
                ch2[idx][0] = glth_px1500::min_mvolts + (glth_px1500::max_mvolts) * (work_rec->fDataPtr[idx]) / (2 << glth_px1500::n_bits);
                ch1[idx][1] = 0.;
            }

            return glth_const::io_read_ok;
        }

        // At present, all we've got is EOF in terms of why reading could
        // have failed.
        else
            return glth_const::io_eof;
    }
}

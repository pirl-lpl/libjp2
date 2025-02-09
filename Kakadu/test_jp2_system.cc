#include <iostream>
using std::cout, std::endl;

#include <string>
using std::string;


//	Kakadu
#include	"JP2_File_Reader.hh"

int main (int argc, char  *argv[])
{

    std::string path(argv[1]);
    UA::HiRISE::Kakadu::JP2_File_Reader jp2reader(path);
    jp2reader.metadata_parameters();
    cout << jp2reader.validity_report() << endl;
}


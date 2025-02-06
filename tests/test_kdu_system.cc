#include <iostream>
using std::cout, std::endl;

//	Kakadu
#include	"kdu_arch.h"

int main (int argc, char  *argv[])
{
    //	Host system.
    cout << "Available threads = " << kdu_core::kdu_get_num_processors () << endl;
}


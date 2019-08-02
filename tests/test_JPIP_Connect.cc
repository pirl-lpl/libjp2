#include <string>
#include <iostream>
using namespace std;

#include "JP2_Exception.hh"
using namespace UA::HiRISE;

#include "Kakadu/JP2_JPIP_Reader.hh"
using namespace UA::HiRISE::Kakadu;

int main (int argc, char  *argv[])
{
   if (argc == 1)
   {
      cerr << "Usage: " << argv[0] << " <URL> " << endl;
      exit(1);
   }

   string url = string(argv[1]);

   cerr << "Checking " << url << endl;

   try
   {
      JP2_JPIP_Reader reader = JP2_JPIP_Reader();
   
      reader.open(url);

      cerr << reader.connection_status() << endl;

      reader.close(true);
   }
   catch(JPIP_Exception ex)
   {
      cout << ex.message() << endl;
      exit(2);
   }

}


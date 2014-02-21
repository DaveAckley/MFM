#include "spiketests.h"

int main(int argc, char** argv)
{
  return !(

	   TestNonBlockingRead() && 
	   TestBlockingRead()

	   );
}

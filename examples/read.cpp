// Simple example for reading data in a non-continuous fashion.

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <array>
#include <chrono>
#include <iostream>
#include <vector>
#include <system_error>
#include <thread>

#include <libsmu/libsmu.hpp>

#ifndef _WIN32
int (*_isatty)(int) = &isatty;
#endif

using std::cout;
using std::cerr;
using std::endl;

using namespace smu;

int main(int argc, char **argv)
{
	// Create session object and add all compatible devices them to the
	// session. Note that this currently doesn't handle returned errors.
	Session* session = new Session();
	session->add_all();

	if (session->m_devices.size() == 0) {
		cerr << "Plug in a device." << endl;
		exit(1);
	}

	// Grab the first device from the session.
	auto dev = *(session->m_devices.begin());

	// Data to be read from the device is formatted into a vector of four
	// floats in an array, specifically in the format
	// <Chan A voltage, Chan A current, Chan B coltage, Chan B current>.
	std::vector<std::array<float, 4>> rxbuf;

	while (true) {
		// Run the session for 1024 samples.
		session->run(1024);
		try {
			// Read 1024 samples at a time from the device.
			// Note that the timeout (3rd parameter to read() defaults to 0
			// (nonblocking mode) so the number of samples returned won't
			// necessarily be 1024.
			dev->read(rxbuf, 1024);
		} catch (const std::system_error& e) {
			if (!_isatty(1)) {
				// Exit on dropped samples when stdout isn't a terminal.
				cerr << "sample(s) dropped: " << e.what() << endl;
				exit(1);
			}
		}

		for (auto i: rxbuf) {
			// Overwrite a singular line if stdout is a terminal, otherwise
			// output line by line.
			if (_isatty(1))
				printf("\r% 6f % 6f % 6f % 6f", i[0], i[1], i[2], i[3]);
			else
				printf("% 6f % 6f % 6f % 6f\n", i[0], i[1], i[2], i[3]);
		}
	};
}

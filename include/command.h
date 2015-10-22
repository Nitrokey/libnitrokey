#ifndef COMMAND_H
#define COMMAND_H
#include <string>
#include "command_id.h"
#include "cxx_semantics.h"

namespace nitrokey {
namespace proto {

template <CommandID cmd_id> 
class Command : semantics::non_constructible {
public:
	constexpr static CommandID command_id() {
		return cmd_id;
	}

	template<typename T>
	static std::string dissect(const T &) {
		return std::string("Payload dissection is unavailable");
	}
};

}
}

#endif

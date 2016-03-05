#ifndef CXX_SEMANTICS_H
#define CXX_SEMANTICS_H

#define __packed __attribute__((__packed__))

/*
 *	There's no need to include Boost for a simple subset this project needs.
 */
namespace semantics {
class non_constructible {
  non_constructible() {}
};
}

#endif

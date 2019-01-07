#include <libnitrokey/NK_C_API.h>
#include "catch2/catch.hpp"
#include <cstring>

TEST_CASE("Issue #137", "[fast]") {
	REQUIRE(NK_login_auto() == 1);
	REQUIRE(NK_user_authenticate("123456", "temppw") == 0);
	auto s = NK_get_totp_code_PIN(0, 0, 0, 0, "temppw");
	REQUIRE(s != 0);
	REQUIRE(strnlen(s, 100) > 0 );
	REQUIRE(NK_logout() == 0);
}

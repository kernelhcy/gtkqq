#include <cprint.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	printf("Test cprint:\n");
	cprintf(GREEN_F, BLUE_B, HIGHLIGHT, "green blue hithlight %s %d\n"
			, "hello", 234);
	cprintf(BLUE_F, UNSET_B, HIGHLIGHT, "blue unset hightlight %s %d\n"
			, "hello", 234);
	cprintf(YELLOW_F, UNSET_B, HIGHLIGHT, "yellow unset  hightlight %s %d\n"
			, "hello", 234);
	cprintf(PURPLE_F, UNSET_B, HIGHLIGHT, "purple unset hightlight %s %d\n"
			, "hello", 234);
	cprintf(GREEN_F, UNSET_B, UNDERLINE, "green unset underline%s %d\n"
			, "hello", 234);
	cprintf(BLUE_F, UNSET_B, BLINKING, "blue unset blinking %s %d\n"
			, "hello", 234);
	cprintf(YELLOW_F, UNSET_B, REVERSE, "yellow unset reverse %s %d\n"
			, "hello", 234);
	cprintf(PURPLE_F, UNSET_B, HIDE, "purple unset hide %s %d\n"
			, "hello", 234);
	return 0;
}

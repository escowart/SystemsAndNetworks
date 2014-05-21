#include "test3600fs.h"
#include "disk.h"

/** Run the tester program for 3600mkfs.c & 3600fs.c */
int main(int argc, char** argv) {
	USE(argc);
	USE(argv);
  dcreate_connect();

	//printf("%lu\n", sizeof(dnode));

	testmount();


  dunconnect();
	return 0;
}

/* Test the mounting of the vcd, the root, and the node */
void testmount() {
	pid_t pid0 = fork();
	if (pid0 < 0) {
    printf("Error: Fork Failed \n");
	} else if (pid0 == 0) {
    char* arg = "3600mkfs";
    char* argv[] = {arg, NULL};
    execvp(arg, arv);
    printf("Error: Execvp Failed");
  } else {
    waitpid(pid0);
    char block0[512];
    dread(0, vcb);
    vcb vcb0 = (vcb*)block0;
    assert(vcb0.magic == MAGICNUM);
  }
}

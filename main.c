#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>

#ifndef GRADING
#define TEST_SYMBOL1 "TEST_SYMBOL1"
#define TEST_SYMBOL2 "TEST_SYMBOL2"
#define TEST_SYMBOL3 "TEST_SYMBOL3"
#define TEST_SYMBOL4 "TEST_SYMBOL4"
#define TEST_DRIVERS "TEST_DRIVER_LOCATION"
#endif

#define INSDUMP_SYSCALL_NUMBER 359
#define RMDUMP_SYSCALL_NUMBER 360

/*Thread function to open a module driver name and perform operations*/
void * open_module_driver(void *arg){
    int fd, remove_dump_status;
    int syscall_id = (int) arg;
    /*Opening a sample module driver to make a call to a function*/
    fd = open("/dev/RBprobe", O_RDWR);

    if(fd < 0){
        printf("Cannot Open  probe Device. Exiting....\n");
    }else{

        printf("Opened the file. \n");
        #ifdef  SYS_gettid
        printf("About to removed the dynamic_dump from the thread function(%ld) with id: %d \n",syscall(SYS_gettid), syscall_id);
        #endif
        /*Failure case where we try to remove the dynamic_dump from the process which has not created it
        * Expecting a -1 as the dump_removal_status.
        */
        remove_dump_status = syscall(RMDUMP_SYSCALL_NUMBER, syscall_id);
        printf("Dump removal status: %d \n",remove_dump_status);
    }
    /*Thread exit once the process is done*/
    pthread_exit(NULL);
}

int main(){
    /*tid of the thread to be created*/
    pthread_t thread_dev1;

    int syscall_return;
    /*return_value*/
    int ret;
    /*check the dump_removal_status*/
    int dump_removal_status;
    /*variable to store the fork id*/
    int fork_id;

    /*Forking to create a child process*/
    fork_id = fork();

    /*Check if the process if forked as expected*/
    if (fork_id < 0) {
        printf("Forking the process failed \n");
    }
    else if (fork_id == 0) {
        printf("Child Process created \n");

        /*Syscall to add the TEST_SYMBOL1 with child process access*/
        syscall_return = syscall(INSDUMP_SYSCALL_NUMBER,TEST_SYMBOL2,0);
        if(syscall_return > 0){
            printf("Dynamic dump created from child process(%d) with id: %d \n", getpid(), syscall_return);
             if((ret = pthread_create(&thread_dev1, NULL, &open_module_driver, (void *)syscall_return))){
                printf("Error creating the thread \n");
            }
        }else{
            printf("Error creating the dynamic dump from child process(%d)\n", getpid());
        }


        /*Waiting for the thread to complete the operation*/
        if(syscall_return > 0 && !ret){
            pthread_join(thread_dev1, NULL);
        }
        /*Success case where the process which added the dump stack is trying to remove it
        *Expect 0 as the dump_removal_status
        */
        printf("About to remove the dynamic_dump from the created child process(%d) with id: %d \n", getpid(), syscall_return);   
        dump_removal_status = syscall(RMDUMP_SYSCALL_NUMBER, syscall_return);
        printf("Dump removal status: %d \n",dump_removal_status);


        /*Syscall to add the TEST_SYMBOL1 with child process access*/
        syscall_return = syscall(INSDUMP_SYSCALL_NUMBER,TEST_SYMBOL1,1);
        if(syscall_return > 0){
            printf("Dynamic dump created from child process(%d) with id: %d \n", getpid(), syscall_return);
             if((ret = pthread_create(&thread_dev1, NULL, &open_module_driver, (void *)syscall_return))){
                printf("Error creating the thread \n");
            }
        }else{
            printf("Error creating the dynamic dump from child process(%d)\n", getpid());
        }

        /*Waiting for the thread to complete the operation*/
        if(syscall_return > 0 && !ret)
            pthread_join(thread_dev1, NULL);

    }
    else {
        printf("Inside parent procees\n");

        /*Syscall to add the TEST_SYMBOL4 with invalid symbol*/
        syscall_return = syscall(INSDUMP_SYSCALL_NUMBER,TEST_SYMBOL4,0);
        if(syscall_return > 0){
            printf("Dynamic dump created from parent process(%d) with id: %d \n", getpid(), syscall_return);
        }else{
            printf("Error creating the dynamic dump from parent process(%d) \n", getpid());
        }

        /*Syscall to add the TEST_SYMBOL3 with owner process access*/
        syscall_return = syscall(INSDUMP_SYSCALL_NUMBER,TEST_SYMBOL3,2);
        if(syscall_return > 0){
            printf("Dynamic dump created from parent process(%d) with id: %d \n", getpid(), syscall_return);
        }else{
            printf("Error creating the dynamic dump from parent process(%d) \n", getpid());
        }
        wait(NULL);
    }

    return 0;
}
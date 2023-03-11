# EasyPThread

An educational real-time C library for easy time & task management using [pthreads](https://en.wikipedia.org/wiki/Pthreads).

<p align="center">
	<a href="#prerequisite">Prerequisite</a>
	<span> • </span>
	<a href="#getting-started">Getting Started</a>
	<span> • </span>
	<a href="#library-functions">Library Functions</a>
	<span> • </span>
	<a href="#example">Example</a>
</p>


## <a id="prerequisite"></a>Prerequisite

* [CMake 3.1+](https://cmake.org) - A cross-platform, open-source build system.


## <a id="getting-started"></a>Getting Started

To get started with EasyPThread, follow these simple steps:

1. Clone the repository:

	``` bash
	git clone https://github.com/vbisog/lib-easypthread
	```

2. Navigate to the repository directory:
	```bash
	cd lib-easypthread
	```

3. Run the installation script / Compile the library by running the installation script in the root directory of the repository:
	```bash
	./build_lib.sh
	```
	This will generate the library file `libeasypthread.a` in the build directory.

4. Include the header files `easy_pthread_time.h` and `easy_pthread_task.h` in your project source code:
	```c
	#include "easy_pthread_time.h"
	#include "easy_pthread_task.h"
	```

5. Link your project with the libeasypthread.a library file by passing the following flag to the compiler:
	```bash
	-L/path/to/lib-easypthread/build -libeasypthread
	```
where /path/to/lib-easypthread/build is the path to the build directory of the EasyPThread repository.

6. Use the EasyPThread library functions in your code. See the [Library Functions](#library-functions) section for more information on available functions.

The [Tanks Simulator](https://github.com/vbisog/tanksimulator) project provides an example of how EasyPThread can be used in a concrete case.

<p align="center" width="100%">
    <img width="61.8%" src="img/rt.png"> 
</p>


## <a id="library-functions"></a>Library Functions

### Time-Related Functions (`easy_pthread_time.h`)

* __time_copy__ 
	```c
	void time_copy(struct timespec *td, struct timespec ts)
	```
	Copies a source time variable `ts` to a destination time variable pointed by `td`.
	* Parameters
		* `td` A pointer to a struct timespec variable representing the destination time.
		* `ts` A struct timespec variable representing the source time.


* __time_add_ms__
	```c
	void time_add_ms(struct timespec *t, int ms)
	```
	Adds a value ms (expressed in milliseconds) to the time variable pointed by `t`.
	* Parameters
		* `t` A pointer to a struct timespec variable representing the time to be incremented.
		* `ms` An integer representing the value to be added to the time variable in milliseconds.

		
* __time_cmp__
	```c
	int time_cmp(struct timespec t1, struct timespec t2)
	```
	Compares two time variables `t1` and `t2` and returns the result of the comparison.
	* Parameters
		* `t1` A struct timespec variable representing the first time.
		* `t2` A struct timespec variable representing the second time.
	* Returns
		* An integer representing the result of the comparison:
			* 0 if t1 = t2
			* 1 if t1 > t2
			* -1 if t1 < t2


* __timespec2ms__
	```c
	double timespec2ms(struct timespec t)
	```
	Converts a timespec structure variable `t` to milliseconds.
	* Parameters
		* `t` A struct timespec variable representing the time to convert.
	* Returns
		* A double value representing the time in milliseconds.


* __get_time_diff_in_ms__
	```c
	double get_time_diff_in_ms(struct timespec *t1, struct timespec *t2)
	```
	Computes the difference between two timespec structure variables `t1` and `t2`, both expressed in milliseconds.
	* Parameters
		* `t1` A pointer to a struct timespec variable representing the first time.
		* `t2` A pointer to a struct timespec variable representing the second time.
	* Returns
		* A double value representing the time difference in milliseconds.


### Task-Related Functions (`easy_pthread_task.h`)


## <a id="example"></a>An example

A use case (taken from [Tanks Simulator](https://github.com/vbisog/tanksimulator)):

```console
foo@bar:~$

# [...] other task WCET estimations [...]

******************************************
************ WCET ESTIMATION: ************
************  Graphics Task   ************
******************************************
Loop length:			82671 cycles
Deadline misses:		2
NOTE: Times are expressed in milliseconds.
Response time analysis:
	Average (AORT):		1,820858
	Maximum (WORT):		34,920520
	Minimum (BORT):		1,273668
	Total:			150532,141663
	Standard deviation:	0,464532
	Utilization factor:	4,552145%
20 worst response times
	1)	rt[82616]:	34,920520
	2)	rt[79819]:	24,135712
	3)	rt[57319]:	22,416414
	4)	rt[25819]:	12,704050
	5)	rt[63808]:	9,537036
	6)	rt[80760]:	9,517341
	7)	rt[82587]:	9,319828
	8)	rt[58991]:	8,379423
	9)	rt[70820]:	8,252726
	10)	rt[20291]:	8,187747
	11)	rt[63319]:	8,085694
	12)	rt[42803]:	8,038885
	13)	rt[46521]:	7.970052
	14)	rt[69319]:	7,946617
	15)	rt[68261]:	7,890043
	16)	rt[54126]:	7,748464
	17)	rt[50348]:	7.742843
	18)	rt[50717]:	7,734707
	19)	rt[78320]:	7,603784
	20)	rt[61391]:	7,584347



******************************************
*****        TANKS SIMULATOR:        *****
******************************************
Total elapsed time:			3306,919036 s

Utilization factor:			5,17%

Deadline misses:
		Task 1	(Tank[1]):	0
		Task 2	(Tank[2]):	0
		Task 3	(Tank[3]):	0
		Task 4	(Tank[4]):	0
		Task 5	(Tank[5]):	0
		Task 6	(Sensor[1]):	0
		Task 7	(Sensor[2]):	1
		Task 8	(Sensor[3]):	0
		Task 9	(Sensor[4]):	0
		Task 10	(Sensor[5]):	0
		Task 11	(User):		1
		Task 12	(Graphics):	2

```

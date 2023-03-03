# __MyPtask Library__

### _Author_: Livio Bisogni
###### __&copy; 2021 REAL-TIME INDUSTRY Inc.__
___
An educational real-time library for stress-free time & task management.

## Prerequisites

* [CMake 3.1+](https://cmake.org) - A cross-platform, open-source build system.

## How to compile

To compile the library the first time:

1. Move the `myptask-main` folder (from now on referred to as `myptask-main/`) where thou prefer.
2. Launch a terminal window and navigate to `myptask-main/`.
3. Type:

    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

## How to use

The compilation produces the library file `myptask-main/build/libmyptask.a`, which must be included in thy desired projects. Adjust the library to thy choosing.
A project that uses it is the [Tanks Simulator](https://github.com/vbisog/tanksimulator). In that repository, there is a [pdf](https://github.com/vbisog/tanksimulator/blob/main/project-report.pdf), which briefly describes the project. Indeed, it describes some of the main aspects of this library, applied to a concrete case. Italian only, though (sorry folks).

![](img/rt.png)

* __time_copy__ 
	```c
	void time_copy(struct timespec *td, struct timespec ts)
	```
	Copies a source time variable ts to a destination time variable pointed by td.
	* Parameters
		* `td`: A pointer to a struct timespec variable representing the destination time.
		* `ts`: A struct timespec variable representing the source time.


* __time_add_ms__
	```c
	void time_add_ms(struct timespec *t, int ms)
	```
	Adds a value ms (expressed in milliseconds) to the time variable pointed by t.
	* Parameters
		* `t`: A pointer to a struct timespec variable representing the time to be incremented.
		* `ms`: An integer representing the value to be added to the time variable in milliseconds.

		
* __time_cmp__
	```c
	int time_cmp(struct timespec t1, struct timespec t2)
	```
	Compares two time variables t1 and t2 and returns the result of the comparison.
	* Parameters
		* t1: A struct timespec variable representing the first time.
		* t2: A struct timespec variable representing the second time.
	* Returns
		* An integer representing the result of the comparison:
			* 0 if t1 = t2
			* 1 if t1 > t2
			* -1 if t1 < t2


* __timespec2ms__
	```c
	double timespec2ms(struct timespec t)
	```
	Converts a timespec structure variable t to milliseconds [ms].
	* Parameters
		* `t`: A struct timespec variable representing the time to convert.
	* Returns
		* A double value representing the time in milliseconds.


* __get_time_diff_in_ms__
	```c
	double get_time_diff_in_ms(struct timespec *t1, struct timespec *t2)
	```
	Computes the difference between two timespec structure variables t1 and t2, both expressed in milliseconds [ms].
	* Parameters
		* `t1`: A pointer to a struct timespec variable representing the first time.
		* `t2`: A pointer to a struct timespec variable representing the second time.
	* Returns
		* A double value representing the time difference in milliseconds.



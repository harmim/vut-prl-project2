/**
 * VUT FIT PRL 2020 Project - Odd-Even Transposition Sort.
 *
 * @author Dominik Harmim <harmim6@gmail.com>
 */


#include <cstdlib>
#include <iostream>
#include <fstream>
#include <mpi.h>


#define COMM MPI_COMM_WORLD /// Default MPI communicator.
#define TAG 0 /// An MPI tag used for transmission of messages with numbers.
#define MASTER 0 /// A rank of the master process.
#define NUMBERS_FILE "numbers" /// A name of the input file with numbers.


using namespace std;


/**
 * Prints an error message due to an MPI error and terminates the program with
 * an erroneous exit code.
 */
auto MPI_error() -> void
{
	cerr << "Error: an MPI library call has failed." << endl;
	MPI_Abort(COMM, EXIT_FAILURE);
}


/**
 * The master process reads numbers from the input file, prints them to
 * standard output, and sends them to the others processes.
 *
 * @throws runtime_error
 * @param rank A rank of a process.
 * @return Returns the number of numbers that have been read.
 */
auto read_numbers(const int rank) -> size_t
{
	if (rank != MASTER)
	{
		return 0;
	}

	ifstream numbers(NUMBERS_FILE);
	if (!numbers.is_open() || numbers.bad())
	{
		cerr << "Error: could not read the input file." << endl;
		MPI_Abort(COMM, EXIT_FAILURE);
	}

	size_t numbers_count = 0;
	for (int r = MASTER; !numbers.eof(); numbers_count = ++r)
	{
		const int number = numbers.get();

		if (numbers.eof())
		{
			break;
		}
		else if (r != MASTER)
		{
			cout << " ";
		}
		cout << number;

		if (MPI_Send(&number, 1, MPI_INT, r, TAG, COMM))
		{
			MPI_error();
		}
	}
	cout << endl;

	return numbers_count;
}


/**
 * Receives a number from the master process.
 *
 * @return Returns the received number.
 */
auto receive_number() -> int
{
	int number;
	if (MPI_Recv(&number, 1, MPI_INT, MASTER, TAG, COMM, nullptr))
	{
		MPI_error();
	}

	return number;
}


auto ots_sort_cmp(
	int &number, const int rank, const int limit, const bool odd
) -> void
{
	if (rank > limit || (odd && rank == MASTER))
	{
		return;
	}

	// even/odd processes send their value to odd/even processes
	if (!odd && rank < limit)
	{
		if (MPI_Send(&number, 1, MPI_INT, rank + 1, TAG, COMM))
		{
			MPI_error();
		}
		if (MPI_Recv(&number, 1, MPI_INT, rank + 1, TAG, COMM, nullptr))
		{
			MPI_error();
		}
	}
	else // odd/even processes compare their value with even/odd processes
	{
		int neigh;
		if (MPI_Recv(&neigh, 1, MPI_INT, rank - 1, TAG, COMM, nullptr))
		{
			MPI_error();
		}
		if (neigh > number) // swap
		{
			MPI_Send(&number, 1, MPI_INT, rank - 1, TAG, COMM);
			number = neigh;
		}
		else
		{
			MPI_Send(&neigh, 1, MPI_INT, rank - 1, TAG, COMM);
		}
	}
}


auto ots_sort(int &number, const int rank, const int procs_count) -> void
{
	int odd_limit = 2 * (procs_count / 2) - 1;
	int even_limit = 2 * ((procs_count - 1) / 2);

	for (int i = 0; i <= procs_count / 2; i++)
	{
		// odd processes compare
		ots_sort_cmp(number, rank, odd_limit, rank % 2);
		// even processes compare
		ots_sort_cmp(number, rank, even_limit, !(rank % 2));
	}
}


/**
 * Receives numbers from all processes.
 *
 * @param numbers Received numbers of all processes.
 * @param number A number of a process.
 * @param rank A rank of a process.
 * @param procs_count The number of all processes.
 */
auto receive_all_numbers(
	int *const numbers, const int number, const int rank, const int procs_count
) -> void
{
	if (rank == MASTER)
	{
		numbers[rank] = number;
		for (int r = MASTER + 1; r < procs_count; r++)
		{
			if (MPI_Recv(&numbers[r], 1, MPI_INT, r, TAG, COMM, nullptr))
			{
				MPI_error();
			}
		}
	}
	else
	{
		if (MPI_Send(&number, 1, MPI_INT, MASTER, TAG, COMM))
		{
			MPI_error();
		}
	}
}


/**
 * Prints given numbers to the standard output.
 *
 * @param numbers Numbers to be printed.
 * @param rank A rank of a process.
 * @param procs_count The number of all processes.
 */
auto print_numbers(
	const int *const numbers, const int rank, const int procs_count
) -> void
{
	if (rank != MASTER)
	{
		return;
	}

	for (int r = MASTER; r < procs_count; r++)
	{
		cout << numbers[r] << endl;
	}
}


auto main(int argc, char *argv[]) -> int
{
	if (MPI_Init(&argc, &argv))
	{
		MPI_error();
	}

	int rank;
	if (MPI_Comm_rank(COMM, &rank))
	{
		MPI_error();
	}

	const size_t numbers_count = read_numbers(rank);
	if (!numbers_count && rank == MASTER)
	{
		MPI_Abort(COMM, EXIT_SUCCESS);
	}

	int procs_count;
	if (MPI_Comm_size(COMM, &procs_count))
	{
		MPI_error();
	}

	int number = receive_number();
	ots_sort(number, rank, procs_count);

	int *const numbers = new int[procs_count];
	receive_all_numbers(numbers, number, rank, procs_count);

	print_numbers(numbers, rank, procs_count);
	delete[] numbers;

	MPI_Finalize();

	return EXIT_SUCCESS;
}

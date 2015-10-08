#include "SortingAlgorithm.h"
#include "sortcommon.h"
#include <vector>

void SortingAlgorithm::getOptions(std::string & options) const
{
	options.clear();
}

void SortingAlgorithm::printID() const
{
	printf("Unidentified algorithm\n");
}

bool SortingAlgorithm::checkOutput(int n, const data_t * in, const data_t * out) const
{
	return checkOutputFullSorted(n, in, out);
}

bool SortingAlgorithm::checkOutputFullSorted(int n, const data_t * in, const data_t * out) const
{
	bool ok = true;
	data_t a;
	setKey(a, 0);
	setValue(a, 0);
	std::vector<data_t> expected(n, a);
	size_t sz = n * sizeof(data_t);
	memcpy(&(expected[0]), in, sz);
	std::sort(expected.begin(), expected.end());

	// Check result
	for (int i = 0; i<n; i++)
	{
		if (getKey(out[i]) != getKey(expected[i]))
		{
			printf("Invalid output, I=%d\n", i);
			for (int j = i - 10; j <= i + 10; j++)
			{
				if (j<0 || j >= n) continue;
				printf("OUT[%3d] = %9u,%4u  EXPECTED[%3d] = %9u,%4u\n", j, getKey(out[j]), getValue(out[j]), j, getKey(expected[j]), getValue(expected[j]));
			}
			ok = false;
			break;
		}
	}

	return ok;
}

bool SortingAlgorithm::checkOutputBlockSorted(int n, int wg, const data_t * in, const data_t * out) const
{
	bool ok = true;
	data_t a;
	setKey(a, 0);
	setValue(a, 0);
	std::vector<data_t> expected(n, a);
	size_t sz = n * sizeof(data_t);
	memcpy(&(expected[0]), in, sz);
	for (int i = 0; i<n; i += wg)
	{
		std::sort(expected.begin() + i, expected.begin() + i + wg);
	}

	// Check result
	for (int i = 0; i<n; i++)
	{
		if (getKey(out[i]) != getKey(expected[i]))
		{
			printf("Invalid output, I=%d\n", i);
			for (int j = i - 10; j <= i + 10; j++)
			{
				if (j<0 || j >= n) continue;
				printf("OUT[%3d] = %9u,%4u  EXPECTED[%3d] = %9u,%4u\n", j, getKey(out[j]), getValue(out[j]), j, getKey(expected[j]), getValue(expected[j]));
			}
			ok = false;
			break;
		}
	}

	return ok;
}
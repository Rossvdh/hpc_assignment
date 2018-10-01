// OpenMP 4.0 allows user - defined reductions using #pragma omp declare reduction. The code above can be simplified with to

#pragma omp declare reduction (merge : std::vector<int> : omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()))

std::vector<int> vec;
#pragma omp parallel for reduction(merge: vec)
for (int i = 0; i < 100; i++) {
	vec.push_back(i);
}
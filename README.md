# carp-job-system

A high-performance thread pool and job system library for the [Carp programming language](https://github.com/carp-lang/Carp), featuring parallel paradigms inspired by Rust's Rayon library.

This library provides POSIX-threads-based concurrency, futures (`async`/`await`), parallel fork-join operations, and parallel collection operators (maps, filters, reductions, etc.), including SIMD-accelerated parallel execution.

---

## Features

- **POSIX Thread Pool**: Lightweight thread pool and work-stealing job queue.
- **Futures & Async/Await**: Simple macro-driven futures interface.
- **Rayon-style Fork-Join**: Run tasks in parallel with `Rayon.join`.
- **Parallel Iterators (`Par`)**:
  - `Par.map`: Parallel map on arrays.
  - `Par.reduce`: Parallel reduction.
  - `Par.for-each!`: Parallel in-place mutation of arrays.
  - `Par.filter`: Parallel filtering of arrays.
  - `Par.find`: Parallel search.
  - `Par.simd-add!`: Multi-core SIMD vector addition (requires `carp-simd`).

---

## Installation

Add this library to your project by loading `rayon.carp` (which transitively loads `jobs.carp`):

```clojure
(load "path/to/carp-job-system/rayon.carp")
```

### Dependencies

- **pthreads**: Requires standard C library thread support (`pthread.h`).
- **carp-simd**: Required for the `Par.simd-add!` vector operations. (Assumes `carp-simd` is located adjacent to this directory or in your load path).

---

## Examples

See [examples.md](examples.md) for usage examples.

---

## Running Tests

To run the unit tests, invoke the test runner with the Carp compiler:

```bash
# Test core Job System and Async/Await
carp -x test/jobs_test.carp

# Test parallel/Rayon-style collection iterators
carp -x test/rayon_test.carp
```

---

## License

This library is licensed under the [MIT License](LICENSE).

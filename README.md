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

## Usage Examples

### 1. Thread Pool Lifecycle
```clojure
(use JobSystem)

(defn main []
  (let [pool (JobSystem.create-pool 4)] ; Create pool with 4 worker threads
    (do
      ; ... do parallel work ...
      (JobSystem.destroy-pool pool)))) ; Clean up pool and join worker threads
```

### 2. Async / Await Futures
Evaluate expressions asynchronously on the thread pool:
```clojure
(let [pool (JobSystem.create-pool 4)
      task1 (async pool (* 2.0f 2.0f))
      task2 (async pool (* 3.0f 3.0f))
      ; Await blocks until the job finishes, returns the value, and frees memory
      v1 (await task1)
      v2 (await task2)]
  (do
    (IO.println &(fmt "Result: %f, %f" v1 v2))
    (JobSystem.destroy-pool pool)))
```

### 3. Fork-Join Concurrency (`Rayon.join`)
Evaluate two expressions concurrently on the pool:
```clojure
(let [pool (JobSystem.create-pool 4)
      pair (Rayon.join pool (* 10 10) (* 20 20))]
  (do
    (IO.println &(fmt "Left: %d, Right: %d" @(Pair.a &pair) @(Pair.b &pair)))
    (JobSystem.destroy-pool pool)))
```

### 4. Parallel Operations (`Par` module)
Apply functional patterns in parallel across chunks of arrays:

```clojure
(defn square-fn [x] (* x x))
(defn gt-five? [x] (> @x 5))
(defn add-fn [a b] (+ a b))

(defn main []
  (let [pool (JobSystem.create-pool 4)
        input [1 2 3 4 5 6 7 8]
        
        ; 1. Parallel Map
        squares (Par.map pool &input &square-fn)
        
        ; 2. Parallel Filter
        filtered (Par.filter pool &input &gt-five?)
        
        ; 3. Parallel Reduce
        sum (Par.reduce pool &input 0 &add-fn)]
    (do
      (IO.println &(fmt "Sum: %d" sum))
      (JobSystem.destroy-pool pool))))
```

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

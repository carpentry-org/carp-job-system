# Examples

## 1. Thread Pool Lifecycle
```clojure
(use JobSystem)

(defn main []
  (let [pool (JobSystem.create-pool 4)] ; Create pool with 4 worker threads
    (do
      ; ... do parallel work ...
      (JobSystem.destroy-pool pool)))) ; Clean up pool and join worker threads
```

## 2. Async / Await Futures
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

## 3. Fork-Join Concurrency (`Rayon.join`)
Evaluate two expressions concurrently on the pool:
```clojure
(let [pool (JobSystem.create-pool 4)
      pair (Rayon.join pool (* 10 10) (* 20 20))]
  (do
    (IO.println &(fmt "Left: %d, Right: %d" @(Pair.a &pair) @(Pair.b &pair)))
    (JobSystem.destroy-pool pool)))
```

## 4. Parallel Operations (`Par` module)
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

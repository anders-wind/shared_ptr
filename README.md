# shared_ptr

Experiments of different shared_ptr implementations. 

Currently there are two different implementations:

- A local not-thread safe `wind::local::shared_ptr`. Structure consist of the pointer to the data and an integer for reference counting.
- A "bias" thread safe `wind::bias::shared_ptr`. Structure consisting of the pointer to the data, an atomic counter for number of threads with copies, and a thread-local counter for number of copies in a thread. This implementation requires support for pthreads.

Important take aways:

- If you do not need thread safety, then `local::shared_ptr` is much faster than the alternatives
- If you share amongst threads and each thread has multiple copies of the ptr then `bias::shared_ptr` is faster.
- If you have a lot of threads and each thread only has one(or a few) copies then `std::shared_ptr` is the fastest. 


# Building and installing

See the [BUILDING](BUILDING.md) document.

But in short:
```
cmake --preset=dev
cmake --build build/dev --config Release -j 24; cmake --install build/dev --config Release --prefix prefix
```

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing

See the [LICENSE](LICENSE) document.

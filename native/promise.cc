

#include <napi.h>
#include "log.h"

#ifdef WIN32
#include <Windows.h>
#endif


#define PRIME_COUNT 10

class PiWorker : public Napi::AsyncWorker
{
public:
    PiWorker(Napi::Env &env)
        : Napi::AsyncWorker(env),
          deferred(Napi::Promise::Deferred::New(env))
    {
        LOG();
    }

    ~PiWorker()
    {
        LOG();
    }

    // Executed inside the worker-thread.
    // It is not safe to access JS engine data structure
    // here, so everything we need for input and output
    // should go on `this`.
    void Execute()
    {
        LOG();
        findPrimeNumbers();

        // you could handle errors as well
        // throw std::runtime_error("test error");
        // or like
        // Napi::AsyncWorker::SetError
        // Napi::AsyncWorker::SetError("test error");
    }

    // Executed when the async work is complete
    // this function will be run inside the main event loop
    // so it is safe to use JS engine data again
    void OnOK()
    {
        LOG();
        // LOG("%d, %d, %d, %d, %d\n", data[0], data[1], data[2], data[3], data[4]);
        // Napi::HandleScope scope(Env());
        // auto num = Napi::Number::New(Env(), 12345);
        // deferred.Resolve(num);
        // return;

        auto arr = Napi::Array::New(Env(), PRIME_COUNT);
        for (int i = 0; i < PRIME_COUNT; ++i)
            arr[i] = Napi::Number::New(Env(), data[i]);

        deferred.Resolve(arr);
    }

    void OnError(Napi::Error const &error)
    {
        LOG();
        deferred.Reject(error.Value());
    }
    Napi::Promise GetPromise() { return deferred.Promise(); }

private:
    Napi::Promise::Deferred deferred;
    int data[PRIME_COUNT] = {
        0,
    };

private:
    void findPrimeNumbers()
    {
        int idx_inner, idx_outer;
        int prime_count = 0;

        // Find the first 1000 prime numbers using an extremely inefficient algorithm.
        for (idx_outer = 2; prime_count < PRIME_COUNT; idx_outer++)
        {
            for (idx_inner = 2; idx_inner < idx_outer; idx_inner++)
            {
                if (idx_outer % idx_inner == 0)
                {
                    break;
                }
            }
            if (idx_inner < idx_outer)
            {
                continue;
            }

            // Save the prime number to the heap. The JavaScript marshaller (CallJs)
            // will free this item after having sent it to JavaScript.
            data[prime_count] = idx_outer;
            ++prime_count;
        }
    }
};

// Asynchronous access to the `Estimate()` function
Napi::Value getPrimeSync(const Napi::CallbackInfo &info)
{
    LOG();
    auto &env = info.Env();
    int data[PRIME_COUNT] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto arr = Napi::Array::New(env, PRIME_COUNT);
    for (int i = 0; i < PRIME_COUNT; ++i)
    {
        arr[i] = Napi::Number::New(env, data[i]);
    }
    return arr; // number[];
}

Napi::Value getPrimeAsync(const Napi::CallbackInfo &info)
{
    LOG();
    auto &env = info.Env();

    PiWorker *piWorker = new PiWorker(env);
    piWorker->Queue();

    return piWorker->GetPromise(); // Promise<number[]>
}

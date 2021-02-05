#include <math.h>
#include <stdlib.h>

#include <libsystem/core/CString.h>
#include <libsystem/process/Process.h>
#include <libsystem/utils/NumberParser.h>

void abort()
{
    process_abort();
}

int atoi(const char *nptr)
{
    int value;
    parse_int(PARSER_DECIMAL, nptr, strlen(nptr), &value);
    return value;
}

long int strtol(const char *nptr, char **endptr, int base)
{
    int value;
    parse_int({base}, nptr, (uintptr_t)endptr - (uintptr_t)nptr, &value);
    return value;
}

long long int strtoll(const char *nptr, char **endptr, int base)
{
    int value;
    parse_int({base}, nptr, (uintptr_t)endptr - (uintptr_t)nptr, &value);
    return value;
}

char *getenv(const char *name)
{
    __unused(name);
    // Pretend the environment variable doesn't exist
    return NULL;
}

void __no_return exit(int status)
{
    process_exit(status);
}

int system(const char *command)
{
    int pid = -1;
    auto result = process_run(command, &pid);

    if (result != SUCCESS)
    {
        return -1;
    }

    int exit_value = -1;
    result = process_wait(pid, &exit_value);

    if (result != SUCCESS)
    {
        return -1;
    }

    return exit_value;
}

double strtod(const char *nptr, char **endptr)
{
    int sign = 1;
    if (*nptr == '-')
    {
        sign = -1;
        nptr++;
    }

    long long decimal_part = 0;

    while (*nptr && *nptr != '.')
    {
        if (*nptr < '0' || *nptr > '9')
        {
            break;
        }
        decimal_part *= 10LL;
        decimal_part += (long long)(*nptr - '0');
        nptr++;
    }

    double sub_part = 0;
    double multiplier = 0.1;

    if (*nptr == '.')
    {
        nptr++;

        while (*nptr)
        {
            if (*nptr < '0' || *nptr > '9')
            {
                break;
            }

            sub_part += multiplier * (*nptr - '0');
            multiplier *= 0.1;
            nptr++;
        }
    }

    double expn = (double)sign;

    if (*nptr == 'e' || *nptr == 'E')
    {
        nptr++;

        int exponent_sign = 1;

        if (*nptr == '+')
        {
            nptr++;
        }
        else if (*nptr == '-')
        {
            exponent_sign = -1;
            nptr++;
        }

        int exponent = 0;

        while (*nptr)
        {
            if (*nptr < '0' || *nptr > '9')
            {
                break;
            }
            exponent *= 10;
            exponent += (*nptr - '0');
            nptr++;
        }

        expn = pow(10.0, (double)(exponent * exponent_sign));
    }

    if (endptr)
    {
        *endptr = (char *)nptr;
    }
    double result = ((double)decimal_part + sub_part) * expn;
    return result;
}

float strtof(const char *nptr, char **endptr)
{
    return strtod(nptr, endptr);
}

double atof(const char *nptr)
{
    return strtod(nptr, nullptr);
}

void *bsearch(
    const void *key,
    const void *base,
    size_t count,
    size_t size,
    int (*compare)(const void *, const void *))
{
    // Invariant: Element is in the interval [i, j).
    size_t i = 0;
    size_t j = count;

    while (i < j)
    {
        size_t k = (j - i) / 2;
        auto element = reinterpret_cast<const char *>(base) + (i + k) * size;
        auto res = compare(key, element);
        if (res < 0)
        {
            j = i + k;
        }
        else if (res > 0)
        {
            i = i + k + 1;
        }
        else
        {
            return const_cast<char *>(element);
        }
    }
    assert(i == j);

    return nullptr;
}

void qsort(void *base, size_t count, size_t size,
           int (*compare)(const void *, const void *))
{
    // TODO: implement a faster sort
    for (size_t i = 0; i < count; i++)
    {
        void *u = (void *)((uintptr_t)base + i * size);
        for (size_t j = i + 1; j < count; j++)
        {
            void *v = (void *)((uintptr_t)base + j * size);
            if (compare(u, v) <= 0)
                continue;

            // swap u and v
            char *u_bytes = (char *)u;
            char *v_bytes = (char *)v;
            for (size_t k = 0; k < size; k++)
            {
                char temp = u_bytes[k];
                u_bytes[k] = v_bytes[k];
                v_bytes[k] = temp;
            }
        }
    }
}
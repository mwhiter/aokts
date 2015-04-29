#pragma once
#pragma pack(push, 1)

#include <vector>

/*
 * so we can use the copy_if function
 */
template <class InputIterator, class OutputIterator, class Predicate>
OutputIterator copy_if(InputIterator first, InputIterator last,
                       OutputIterator result, Predicate pred)
{
  while(first!=last)
  {
    if(pred(*first))
        *result++ = *first;
    ++first;
  }
  return result;
}

#pragma pack(pop)



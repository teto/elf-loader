#include <list>
#include <string.h>
#include <vector>
#include "vdl-array.h"
#include "internal-test.h"

bool test_array (void)
{
  struct VdlArray *a = vdl_array_new (int);
  vdl_array_delete (a);
  a = vdl_array_new (int);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_size (a), 0);
  vdl_array_push_back (a, 10);
  vdl_array_push_front (a, 5);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_size (a), 2);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 0, int), 5);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 1, int), 10);
  vdl_array_push_front (a, 1);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_size (a), 3);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 0, int), 1);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 1, int), 5);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 2, int), 10);
  vdl_array_pop_front (a);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_size (a), 2);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 0, int), 5);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 1, int), 10);
  vdl_array_insert (a, 1, 4);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_size (a), 3);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 0, int), 5);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 1, int), 4);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_at (a, 2, int), 10);
  std::vector<int> tmp;
  for (int *i = (int*)vdl_array_begin(a); 
       i != (int*)vdl_array_end (a); i++)
    {
      tmp.push_back (*i);
    }
  INTERNAL_TEST_ASSERT_EQ (tmp.size (), 3);
  INTERNAL_TEST_ASSERT_EQ (tmp[0], 5);
  INTERNAL_TEST_ASSERT_EQ (tmp[1], 4);
  INTERNAL_TEST_ASSERT_EQ (tmp[2], 10);
  INTERNAL_TEST_ASSERT (!vdl_array_empty (a));
  vdl_array_clear (a);
  INTERNAL_TEST_ASSERT_EQ (vdl_array_size (a), 0);
  INTERNAL_TEST_ASSERT (vdl_array_empty (a));
  vdl_array_delete (a);
  return true;
}

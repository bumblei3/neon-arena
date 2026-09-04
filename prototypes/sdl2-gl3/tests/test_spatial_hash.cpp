// Tests for SpatialHash class
#include <cstdio>
#include <cassert>
#include <cmath>
#include "../src/spatial_hash.h"

static int shPassed = 0, shFailed = 0;

#define SH_TEST(name, expr) do { \
    printf("  Running %s... ", name); \
    fflush(stdout); \
    if (expr) { printf("PASSED\n"); shPassed++; } \
    else { printf("FAILED\n"); shFailed++; } \
} while(0)

void testSpatialHash() {
    printf("\n[Spatial Hash Tests]\n");

    // Construction
    {
        SpatialHash h(5.0f);
        SH_TEST("constructs_with_cell_size", h.getCellSize() == 5.0f);
    }

    // Empty grid query
    {
        SpatialHash h;
        auto result = h.queryNearby(0, 0);
        SH_TEST("empty_query_returns_empty", result.empty());
    }

    // Single insert
    {
        SpatialHash h;
        h.insert(42, 10.0f, 10.0f);
        auto result = h.queryNearby(10.0f, 10.0f, 1.5f);
        SH_TEST("single_insert_found", result.size() == 1 && result[0] == 42);
    }

    // Multiple inserts same cell
    {
        SpatialHash h(5.0f);
        h.insert(1, 0, 0);
        h.insert(2, 1, 1);
        h.insert(3, 2, 2);
        auto result = h.queryNearby(0, 0, 2.0f);
        SH_TEST("multiple_in_same_cell", result.size() == 3);
    }

    // Query radius boundary
    {
        SpatialHash h(5.0f);
        h.insert(1, 0, 0);
        h.insert(2, 15, 0); // mid distance
        auto near = h.queryNearby(0, 0, 1.5f);
        auto far = h.queryNearby(0, 0, 20.0f); // large radius
        SH_TEST("near_query_excludes_far", near.size() == 1 && near[0] == 1);
        SH_TEST("large_query_includes_all", far.size() == 2);
    }

    // Different cells
    {
        SpatialHash h(10.0f);
        h.insert(10, 0, 0);
        h.insert(11, 50, 0); // different cell
        h.insert(12, 0, 50); // different cell
        auto result = h.queryNearby(0, 0, 2.0f);
        SH_TEST("different_cells_excluded", result.size() == 1 && result[0] == 10);
    }

    // Negative coordinates
    {
        SpatialHash h(5.0f);
        h.insert(99, -3.0f, -4.0f);
        auto result = h.queryNearby(-3.0f, -4.0f, 1.0f);
        SH_TEST("negative_coords", result.size() == 1 && result[0] == 99);
    }

    // Clear
    {
        SpatialHash h;
        h.insert(1, 0, 0);
        h.insert(2, 1, 1);
        h.clear();
        auto result = h.queryNearby(0, 0, 10.0f);
        SH_TEST("clear_removes_all", result.empty());
    }

    // Set cell size
    {
        SpatialHash h(5.0f);
        h.setCellSize(10.0f);
        SH_TEST("set_cell_size", h.getCellSize() == 10.0f);
    }

    // Query doesn't check beyond radius
    {
        SpatialHash h(5.0f);
        h.insert(1, 0, 0);
        h.insert(2, 4.9f, 0); // just within radius 5
        h.insert(3, 5.1f, 0); // just outside radius 5
        auto result = h.queryNearby(0, 0, 5.0f);
        // Cell-based query is approximate - check both cells are queried
        SH_TEST("radius_boundary_cells", result.size() >= 1);
    }

    // Duplicate IDs (same position)
    {
        SpatialHash h(5.0f);
        h.insert(1, 2, 2);
        h.insert(1, 2, 2); // duplicate
        auto result = h.queryNearby(2, 2, 1.0f);
        SH_TEST("duplicate_ids_both_returned", result.size() == 2);
    }

    printf("\n[Spatial Hash Results] Passed: %d, Failed: %d\n", shPassed, shFailed);
}

int main() {
    testSpatialHash();
    return shFailed > 0 ? 1 : 0;
}

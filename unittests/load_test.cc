#include "internal/Tpch.h"
#include "internal/BaseTypes.h"
#include "internal/File.h"
#include "runtime/Runtime.h"

#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <unordered_set>

TEST(LOAD_STRING, REGIONS_READ_CORRECTLY){
    const std::string dir{"../../data-generator/output/region"};
    ColumnMapping<StringView> page{dir, "r_name"};
    std::vector<std::string_view> regions{
        "AFRICA",
        "AMERICA",
        "ASIA",
        "EUROPE",
        "MIDDLE EAST"
    };
    size_t i = 0;
    StringView sV;
    for(auto& sv: regions){
        load_from_slotted_page(i++, &page, &sV);
        std::string_view region{sV.data, sV.length};
        EXPECT_EQ(region, sv);
    }
}

TEST(LOAD_STRING, CORRECT_PAIR){
        const std::string dir{"../../data-generator/output/lineitem"};
    ColumnMapping<char> returnflag{dir, "l_returnflag"};
    ColumnMapping<char> linestatus{dir, "l_linestatus"};
    StringView s1, s2;
    std::unordered_set<std::string> pairs;
    for(size_t i = 0; i < 6001215; ++i){
        pairs.insert(std::string(1, returnflag.data[i]) + std::string(1, linestatus.data[i]));
    }
    EXPECT_EQ(pairs.size(), 4);
}

TEST(LOAD_INTEGER, LINEITEM_READ_CORRECTLY){
    const std::string dir{"../../data-generator/output"};
    InMemoryTPCH db{dir};
    for(size_t i = 0; i < db.region.tuple_count; ++i){
        EXPECT_EQ(db.region.r_regionkey.data[i], i);
    }
}

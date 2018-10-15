// cpdt's simple c++ test framework
#include <iostream>
#include <vector>
struct TestDeclaration {
    const char *name;
    bool (*test)();
    TestDeclaration(const char *name, bool (*test)()) : name(name), test(test) {}
};
struct DescribeDeclaration {
    const char *name;
    std::vector<TestDeclaration> tests;
    DescribeDeclaration(const char *name, std::vector<TestDeclaration> tests) : name(name), tests(std::move(tests)) {}
};
extern std::vector<DescribeDeclaration> declarations;
const char *failedLine;
int numberOfAsserts = 0;
int main() {
    struct FailedTest {
        const char *describe;
        const char *name;
        std::string failedString;
    };
    int testCount = 0;
    int skipCount = 0;
    std::vector<FailedTest> failedTests;
    for (const auto &declaration : declarations) {
        std::cout << declaration.name << std::endl;
        for (const auto &test : declaration.tests) {
            testCount++;
            std::cout << "  " << test.name << " ..";
            numberOfAsserts = 0;
            if (test.test()) {
                if (numberOfAsserts > 0)
                    std::cout << " PASSED" << std::endl;
                else {
                    std::cout << " SKIPPED" << std::endl;
                    skipCount++;
                }
            } else {
                std::cout << " FAILED" << std::endl;
                failedTests.push_back({declaration.name, test.name, std::string(failedLine)});
            }
        }
    }
    std::cout << std::endl;
    if (failedTests.empty()) {
        std::cout << "All " << testCount << " tests passed";
        if (skipCount) std::cout << " (" << skipCount << " skipped)";
        std::cout << std::endl;
        return 0;
    } else {
        std::cout << failedTests.size() << " test(s) out of " << testCount << " failed:" << std::endl;
        for (const auto &failedTest : failedTests) {
            std::cout << "  " << failedTest.describe << " " << failedTest.name << " FAILED" << std::endl;
            std::cout << "    at " << failedTest.failedString << std::endl;
        }
        return 1;
    }
}
#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)
#define TESTS(block) std::vector<DescribeDeclaration> declarations = block;
#define DESCRIBE(name, block) DescribeDeclaration((name), std::vector<TestDeclaration>(block)),
#define IT(name, block) \
    TestDeclaration((name), []() -> bool { \
        block; \
        return true; \
    }),
#define ASSERT(expr) \
    { \
        numberOfAsserts++; \
        if (!(expr)) { \
            failedLine = __FILE__ ":" S__LINE__; \
            return false; \
        } \
    }

#include <memory>

#include "SequenceOperators.h"

struct MoveCopyDetector {
    bool hasMoved = false;

    MoveCopyDetector() {}

    MoveCopyDetector(const MoveCopyDetector &other) {}

    MoveCopyDetector(MoveCopyDetector &&other) noexcept {
        other.hasMoved = true;
        hasMoved = false;
    }

    MoveCopyDetector &operator=(const MoveCopyDetector &other) { return *this; }

    MoveCopyDetector &operator=(MoveCopyDetector &&other) noexcept {
        other.hasMoved = true;
        hasMoved = false;
        return *this;
    }
};

TESTS(
    {DESCRIBE("Sanity", {IT("asserts true", { ASSERT(true); })}) DESCRIBE(
        "BlankSequence", {IT("is empty",
                             {
                                 auto blankSequence = AxiomCommon::blank<std::unique_ptr<int>>();
                                 ASSERT(blankSequence.empty());
                             })}) DESCRIBE("OnceSequence", {IT("has a size of 1",
                                                               {
                                                                   auto onceSequence = AxiomCommon::once<int>(1);
                                                                   ASSERT(onceSequence.size() == 1);
                                                               }) IT("emits the single value provided",
                                                                     {
                                                                         auto onceSequence = AxiomCommon::once<int>(1);
                                                                         ASSERT(*onceSequence.begin() == 1);
                                                                         ASSERT(++onceSequence.begin() ==
                                                                                onceSequence.end());
                                                                     })})
         DESCRIBE("FilterMapGenerator", {IT("has a size equal to the input", {
                      std::vector<int> input({0, 1, 2, 3, 4, 5});
                      auto filterMapSequence =
                          AxiomCommon::filterMap(std::move(input), [](int input) { return std::make_optional(input); });
                      ASSERT(filterMapSequence.size() == 6);
                  }) IT("uses the provided function to filter and map to the output", {
                      std::vector<int> input({0, 1, 2, 3, 4, 5});
                      auto filterMapSequence = AxiomCommon::filterMap(std::move(input), [](int input) {
                          return input % 2 == 0 ? std::make_optional(input + 1) : std::nullopt;
                      });
                      auto iter = filterMapSequence.begin();
                      ASSERT(*iter == 1);
                      ASSERT(*++iter == 3);
                      ASSERT(*++iter == 5);
                      ASSERT(++iter == filterMapSequence.end());
                  }) IT("moves values out of the input", {
                      std::vector<MoveCopyDetector> input({MoveCopyDetector(), MoveCopyDetector(), MoveCopyDetector()});
                      auto filterMapSequence = AxiomCommon::filterMap(std::move(input), [](MoveCopyDetector input) {
                          return std::make_optional(std::move(input));
                      });

                      // use size to iterate over the entire sequence
                      filterMapSequence.size();

                      // make sure everything's been moved
                      for (const auto &item : filterMapSequence) {
                          ASSERT(item.hasMoved);
                      }
                  })})})

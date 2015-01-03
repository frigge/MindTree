#ifndef INIT_HN78IJ70

#define INIT_HN78IJ70

namespace MindTree
{
    void initApp(int argc, char *argv[]);
    void finalizeApp();
    void initGui();
    void parseArguments(int argc, char* argv[]);

    void runTests(std::vector<std::string> testlist);

    bool noGui();

namespace {
    bool nogui;
}
} /* MindTree */

#endif /* end of include guard: INIT_HN78IJ70 */

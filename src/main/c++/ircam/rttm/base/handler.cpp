//
//  handler.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 28/08/13.
//  Copyright (c) 2013 Clement Poncelet. All rights reserved.
//

#include <base/handler.h>

//Version Management
void version(ostream &out)
{
    out << "Timed trace checker V0" << endl;
}

//From Antescofo by JL Giavitto
void error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);  // initialize va_list

    // So stdout and stderr appears correctly interleaved if redirected on the same io
    fflush(stdout);

    version(cerr);
    std::fprintf(stderr, "- Error: ");
    std::vfprintf(stderr, fmt, ap);
    std::fprintf(stderr, "\n");
    fflush(stderr);

    va_end(ap);

    exit(EXIT_FAILURE);
}

void usage(ostream &out)
{
    out << endl
    << "Usage : timedtracechecker -(V|T) ..." << endl
    << "  [mode] Other options depend of the mode" << endl
    << "  For Translation: Uppaal-trace, score and table are needed" << endl
    << "  For Verification: reference, the Antescofo output to test and the symbol table are needed" << endl
    << endl;

    out << "Modes: " << endl
    << " -T:  Translation, the trace (from Uppaal) will be translated according the options" << endl
    << " \n\t The T options are:" << endl
    << " --utrace(-u): The Uppaal relative trace to translate." << endl
    << " --score (-s): The mixed-score (the mixed-score corresponding to the tests) " << endl
    << " --rel: do not apply a tempo curve on input traces (relative input and reference trace)" << endl
    << " --treco: create an input trace that is a detected input trace (a result of recognition)"

    << "\n -V:  Verification, timed conformance: test if the real trace (--totest) is conformed to the reference trace (--ref)" << endl
    << " \n\t The V options are:" << endl
    << " --ref (-r): Reference trace (or expected [in physical time])." << endl
    << " --tempo: Checks tempo values too with time values."
    << " --totest: The real Antescofo output trace which has to be conformed to the ref" << endl

    << " \n\t Similar options are:" << endl
    << "    --table (-t): The symbols table that maps ref labels to Antescofo labels." << endl;

    out << "\nOthers options:" << endl
    << " --version (-v) :   current version" << endl
    << " --help (-h) :     print this help" << endl
    << endl;
}


bool check_and_complete_options()
{
    if (!symboltable)       error("option --table (-t) required");
    if(f_verif && f_tran)   error("one mode at the same time");

    if(f_tran)
    {
        run_mode = TRANSLAT;
        if (!scorename)       error("option --score (-s) required");
        if (!uppaaltrace)     error("option --utrace (-u) required");

        if(f_kind && !input_file)   error("option --kind need an input file with this option --tin");

    } else if(f_verif)
    {
        run_mode = VERIF;
        if (!reftrace)       error("option --ref (-r) required");
        if (!realtrace)      error("option --totest required");
    } else                   error("no mode given");

    return false;
}

void set_flag_options(int option_index)
{
    switch(option_index)
    {
        case 9:
            check_index_is("kind");
            //Latter other generation options
            //      TIN for now
            break;

            default:
            ; // nothing for (2,3 and 7)
    }
}

// ==========================================================================|
// Main
int perform(int argc, char *argv[]) {

    if (argc==1)
    {
        usage(std::cerr);
        return EXIT_FAILURE;
    }


    /// Parse command-line options
    int index = 0; // index of the current option
    int c;

    while ((c = getopt_long_only(argc, argv, "hvVTRr:s:t:e:u:a:k:i:p:", longoptions, &index)) != -1)
    {
        switch (c)
        {
            case 0:  // Value returned by the options that set a flag.
                set_flag_options(index);
                break;

            case 'h':
                usage(std::cout);
                exit(EXIT_SUCCESS);

            case 'v':
                version(std::cout);
                exit(EXIT_SUCCESS);

            case 'r':
                reftrace = optarg;
                break;

            case 's':
                scorename = optarg;
                break;

            case 't':
                symboltable = optarg;
                break;

            case 'u':
                uppaaltrace = optarg;
                break;

            case 'a':
                realtrace = optarg;
                break;

            case 'p':
                pathname = optarg;
                break;

            case 'i':
                input_file = optarg;
                break;

            case ':':
                //  missing option argument. Error already signaled by getopt
                error("Missing option argument");
                break;

            case '?':
                // unknown or ambiguous option  Error already signaled by getopt
                error("Warning: ambiguous option argument");
                break;

            default:
                // Do nothing for the options that are not flag-managed and without arguments
                ;
        }
    }

    if (check_and_complete_options())
        std::exit(EXIT_FAILURE);

    int ret = EXIT_FAILURE;
    switch (run_mode)
    {
            // Translation
        case TRANSLAT:
            assert(uppaaltrace && symboltable);
            ret = run_translation();
            break;

            // Verification
        case VERIF:
            assert(reftrace && realtrace && symboltable);
            ret = run_verification();
            break;

        case NONE: exit(EXIT_FAILURE);
    }

    //free ressources
    flush_files();

    std::cout << "finito. ||" << endl;
    exit(ret);
}

int run_translation()
{
    std::cout << "In translation: " << uppaaltrace << std::endl;

    string pathbase(uppaaltrace);
    unsigned long p = pathbase.find_last_of(".");
    pathbase.erase(pathbase.begin()+(long)p, pathbase.end());

    //link real symbols to model labels
    // and checks doublons
    assert(symboltable);
    Symbol_table_parser* symbol_parser;
    map <string, Symbol_phy*>* table;
    symbol_parser = new Symbol_table_parser(symboltable);
    table = symbol_parser->create_table();

    //include pitches chord and poss.
    assert(scorename);
    AntescofoScoreParser score_parser(scorename);
    score_parser.add_pitches(table);

    //Parse coVer trace
    UpppalSuiteParser CoVerParser(uppaaltrace, table);
    while(!CoVerParser.is_eof())
    {
        Trace* testcase = CoVerParser.parseATrace((bool)! f_rel);

        //test debbug
        //Show debug(std::cerr);
        //testcase->accept(&debug);


        if(!f_rel)
        {
         //   WRONG: We cannot loose the action changes ! (model specifications)
         //   TempoCurve* Curve = new Cover_TempoCurve(score_parser.get_score_tempo());
         //   testcase->modify(Curve);

        //TOTEST with tempo curves.
        // tin === treco ?
            TempoCurve* Curve = new TempoCurve(score_parser.get_score_tempo());
            testcase->modify(Curve);
        }

        ///> Input trace to input trace (Before Computing the ref. ! (i.e Cover))

        //TempoCurve Curve(120.0);
        //Cover_TempoCurve Curve(basename+".txt");
        //Equation and Sinus to explore [?] Equation_TempoCurve Curve(60, 4, -40, 100, -70);
        /*    TempoCurve* Curve;
            if(f_kind)
                Curve = new Input_Curve(input_file, score_parser.get_score_tempo());
            else
                Curve = new Random_TempoCurve(score_parser.get_score_tempo());

            testcase->modify(Curve);

            //We print the tempo curve created ::
            //GnuPlotTempoCurve* printcurve = new GnuPlotTempoCurve(pathbase);
            //t1.gettests()->accept(printcurve);

            delete Curve;
//          delete printcurve;
            std::cout << " ... " << endl;
        */

        //test debbug
        //const Testsuite* ts2 = t1.gettests();
        //ts2->accept(new PrintShow(std::cerr));

        // An uppaaltrace is several test cases
        // We give the path/base name of the future input/expected traces
        //      next <argument>.aaa.in are input files created
        //      and  <argument>.aaa.ref are expected files created
        // with aaa an numbers from 1 to N of test cases
        auto last_event = table->end();
        last_event--;
        Print_tin tinPrinter(pathname ? pathname : pathbase, CoVerParser.numberTrace());
        Print_tref trefPrinter(pathname ? pathname : pathbase, CoVerParser.numberTrace(), (last_event)->first);

        testcase->accept(&tinPrinter);  ///> print input trace
        if((bool)! f_rel)
        {
            Computation_Rel_To_Phy_Visitor* translator = new Computation_Rel_To_Phy_Visitor((bool)f_treco,
                                                                                          score_parser.get_score_tempo());
            testcase->modify(translator); ///> translation of the reference if not relative
            delete translator;
        }
        testcase->accept(&trefPrinter); ///> print reference trace
    }

    std::cout << "We have " << CoVerParser.numberTrace() << " trace(s) " << std::endl;

    delete table;
    delete symbol_parser;
    return EXIT_SUCCESS;
}

int run_verification()
{
    std::cout << "... Verification on air ... " << std::endl;
    std::cout << "Is " << realtrace << endl
    << "conform to " << reftrace << endl;


    // Parsing of traces
    //ref
    ReferenceTraceParser ref_parser(reftrace);
    Trace* reference_trace = ref_parser.parse();

    //TODO print ref trace ?

    //real
    AntescofoOutputParser real_parser(realtrace);
    Trace* real_trace = real_parser.parse();

    //link symbols using the table_symbol
    Symbol_table_parser symbol_parser(symboltable);
    symbol_parser.link(reference_trace, real_trace);

    //Verification
    TimedConformanceChecker checker(reference_trace, real_trace, (bool)f_tempo);
    bool res = checker.check();

    //Print for debug
 //   Show* printer = new Show(cerr);
 //   reference_trace->accept(printer);
 //   real_trace->accept(printer);
 //   delete printer;


    /*
     unsigned long under = filename.find_last_of("_", filename.size());
     unsigned long size =  filename.find_last_of(".", filename.size()) - under;
     string fend = filename.substr(under, size);

     GnuPlotVerdict plotVerdict((*basename)+"/"+fend, CheckEngine->getExpt(), CheckEngine->getObtained());
     plotVerdict.plot();
     */

    //free
    delete reference_trace;
    delete real_trace;

    if(res) {
        std::cout << "\tChecked :: Test OK" << std::endl;
        return EXIT_SUCCESS;
    } else {
        std::cout << "\tError :: Test KO" << std::endl;
        return EXIT_FAILURE;
    }
}


void flush_files()
{
    //TODO add input trace file (curves)
    if (input_file) delete input_file;

    if(symboltable) delete symboltable;
    if(pathname) delete pathname;

    //Mode T
    if(f_tran)
    {
        delete scorename;
        delete uppaaltrace;
    }

    //Mode V
    if(f_verif)
    {
        delete reftrace;
        delete realtrace;
    }
}

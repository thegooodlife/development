///////////////////////////////////////////////////////////////////////////////
//
//  model_optimizer.cpp
//
//  Author: John Pazarzis
//
//  Used to optimize and back test a specific strategy
//
//  You need to provide a configuration file containing the required parameters

#include "toolkit.h"
#include "TickEngine.h"
#include <string.h>
#include <string>
#include <iostream>
#include <time.h>
#include "Order.h"
#include "SellBasedInDelta.h"
#include "Logger.h"
#include "Identifiable.h"
#include <fstream>
#include "GeneticAlgorithm.h"
#include "xmldocument.h"
#include "TickPool.h"
#include "ParallelProcessor.h"
#include <sys/resource.h>

using namespace std;

#define NUMBER_OF_THREADS 4

void run_optimizer(XmlNode& config)
{
    try
    {
        const std::string tick_file =  config["tick_file"].value();
        const int colony_size = config["colony_size"].value_to_int();
        assert(config.contains("from_date"));
        auto from_date = make_date(config["from_date"].value());
        assert(config.contains("to_date"));
        auto to_date = make_date(config["to_date"].value());

        if(!from_date.is_not_a_date() && !to_date.is_not_a_date())
        {
            assert(from_date < to_date);
        }

        TickPool& tp = TickPool::singleton();
        cout << timestamp() << endl;
        cout << "loading ticks..." << endl;
        tp.load(tick_file, from_date, to_date);
        cout << timestamp() << endl;
        cout << tp.size() << endl;
        GeneticAlgorithm<SellBasedInDelta> ga(colony_size);
        int i = 0;

        for(;;)
        {
            cout << "generation: " << ++i << " " << cout << timestamp() << endl;
            ParallelProcessor<SellBasedInDelta> pp(&SellBasedInDelta::calculate_fitness, NUMBER_OF_THREADS);

            for(int i = 0; i < ga.size(); ++i)
            {
                pp.add(ga[i]);
            }

            pp.go();

            if(ga.evolve(true))
            {
                break;
            }

            if(!ga.was_rolled_back())
            {
                cout << "Winner so far" << endl;
                cout << ga[0]->get_full_description2() << endl;
            }
        }

        cout << "done" << endl;
    }
    catch(const char* psz)
    {
        std::cout << psz << std::endl;
    }
}

void forward_test2(XmlNode& config)
{
    std::string filename = config["tick_file"].value();
    double minute_to_trade = config["minute_to_trade"].value_to_double();
    double delta = config["delta"].value_to_double();
    double stop_loss = config["stop_loss"].value_to_double();
    double take_profit = config["take_profit"].value_to_double();
    double expriration_minutes = config["expriration_minutes"].value_to_double();
    assert(config.contains("from_date"));
    auto from_date = make_date(config["from_date"].value());
    assert(config.contains("to_date"));
    auto to_date = make_date(config["to_date"].value());

    if(!from_date.is_not_a_date() && !to_date.is_not_a_date())
    {
        assert(from_date < to_date);
    }

    TickPool& tp = TickPool::singleton();
    cout << timestamp() << endl;
    cout << "loading ticks..." << endl;
    tp.load(filename, from_date, to_date);
    cout << timestamp() << endl;
    cout << tp.size() << endl;
    SellBasedInDelta model;
    model.set_values(minute_to_trade, delta, stop_loss, take_profit, expriration_minutes);
    model.export_balance_curve("running_balance.csv");
    model.calculate_fitness();
    cout << "Testing period: from " << from_date << " to " << to_date << endl;
    cout << model.get_full_description2() << endl;
}


void forward_test(XmlNode& config)
{
    std::string filename = config["tick_file"].value();
    double minute_to_trade = config["minute_to_trade"].value_to_double();
    double delta = config["delta"].value_to_double();
    double stop_loss = config["stop_loss"].value_to_double();
    double take_profit = config["take_profit"].value_to_double();
    double expriration_minutes = config["expriration_minutes"].value_to_double();
    assert(config.contains("from_date"));
    auto from_date = make_date(config["from_date"].value());
    assert(config.contains("to_date"));
    auto to_date = make_date(config["to_date"].value());

    if(!from_date.is_not_a_date() && !to_date.is_not_a_date())
    {
        assert(from_date < to_date);
    }

    SellBasedInDelta model;
    model.set_values(minute_to_trade, delta, stop_loss, take_profit, expriration_minutes);
    TickEngine te;
    model.start_listening(&te);
    te.run(filename, from_date, to_date);
    model.calc_fitness(true);
    cout << "Testing period: from " << from_date << " to " << to_date << endl;
    cout << model.get_full_description() << endl;
    Order::clear_order_pool();
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        cout << "correct use: " << argv[0] << "[configuration.xml]" << endl;
        return -1;
    }

    cout << timestamp() << endl;
    srand(time(NULL));
    XmlDocument configuration(argv[1]);

    if(configuration.contains("forward_test"))
    {
        forward_test2(configuration["forward_test"]);
    }
    else  if(configuration.contains("optimize"))
    {
        run_optimizer(configuration["optimize"]);
    }
    else
    {
        cout << "nothing to do.." << endl;
    }

    cout << timestamp() << endl;
    return 0;
}

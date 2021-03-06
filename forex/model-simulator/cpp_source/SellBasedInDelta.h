#ifndef MODELS_INCLUDED
#define MODELS_INCLUDED
#define MODELS_INCLUDED

/////////////////////////////////////////////////////
// SellBasedInDelta.h
//
// Author: John Pazarzis
//
// Date  : Oct. 23 2013
//
// Summary:
//  Simple model based in the high price of the hour and a specific minute
//  when the signal will be orginated
//

#include "Model.h"
#include "CloneableDouble.h"
#include "TickPool.h"
#include "FitnessStatistics.h"
#include <iostream>

class SellBasedInDelta: public Model
{
        CloneableDouble _minute_to_buy;
        CloneableDouble _triggering_delta; // in pips
        CloneableDouble _stop_loss;        // in pips
        CloneableDouble _take_profit;      // in pips
        CloneableDouble _expriration_minutes;      // in pips

        FitnessStatistics _fitness_statistics;

        int _current_hour;
        double _open_price;
        bool _triggered_for_current_hour;
        double _high_for_the_hour;

    protected:
        virtual void initialize_optimizable_fields()
        {
            add_optimizable_field(&_minute_to_buy);
            add_optimizable_field(&_triggering_delta);
            add_optimizable_field(&_stop_loss);
            add_optimizable_field(&_take_profit);
            add_optimizable_field(&_expriration_minutes);
        }

    public:
        SellBasedInDelta():
            _minute_to_buy(20, 60, 1),
            _triggering_delta(2, 12, 2),
            _stop_loss(8, 22, 2),
            _take_profit(10, 22, 2),
            _expriration_minutes(10, 300, 1),
            _current_hour(-1),
            _open_price(0.0),
            _triggered_for_current_hour(false)
        {
            initialize_optimizable_fields();
        }

        void set_values(double minute, double delta, double sl, double pt, double expriration_minutes)
        {
            _minute_to_buy.read_from_double(minute);
            _triggering_delta.read_from_double(delta);
            _stop_loss.read_from_double(sl);
            _take_profit.read_from_double(pt);
            _expriration_minutes.read_from_double(expriration_minutes);
        }

        virtual ~SellBasedInDelta()
        {
        }

        // Exports the balance curve to a flat file so it can later be used
        // for testing and research
        void export_balance_curve(const std::string& filename)
        {
            using namespace std;

            TickPool& tp = TickPool::singleton();
            const int number_of_ticks = tp.size();
            std::vector<Order*> orders;

            for(register int i = 0; i < number_of_ticks; ++i)
            {
                process_tick(tp[i], orders, i);
            }

            
            ofstream balance_file;
            balance_file.open(filename.c_str());
            double accountbalance = 50000;
            balance_file << accountbalance << endl;
            for(auto order_ptr : orders)
            {
                accountbalance += order_ptr->get_pnl();
                balance_file << accountbalance << endl;
                if(accountbalance <= 0)
                    break;
            }

            balance_file.close();

            for(auto order_ptr : orders)
            {
                Order::release(order_ptr);
            }
        }

        void calculate_fitness()
        {
            TickPool& tp = TickPool::singleton();
            const int number_of_ticks = tp.size();
            std::vector<Order*> orders;

            for(register int i = 0; i < number_of_ticks; ++i)
            {
                process_tick(tp[i], orders, i);
            }

            _fitness_statistics = FitnessStatistics::make(orders);;
            set_fitness(_fitness_statistics.fitness());

            for(auto order_ptr : orders)
            {
                Order::release(order_ptr);
            }
        }

        void process_tick(const Tick& tick, std::vector<Order*>& orders, int current_tick_index)
        {
            if((int)tick.timestamp().time_of_day().hours() != _current_hour)
            {
                _current_hour = (int)tick.timestamp().time_of_day().hours();
                _open_price = tick.bid();
                _triggered_for_current_hour = false;
                _high_for_the_hour = tick.bid();
            }
            else
            {
                if(tick.bid() > _high_for_the_hour)
                {
                    _high_for_the_hour = tick.bid();
                }

                if(_triggered_for_current_hour || (int) tick.timestamp().time_of_day().minutes() != (int)_minute_to_buy)
                {
                    return;
                }

                const double delta_in_pips = (_high_for_the_hour - _open_price) * 10000;

                if(delta_in_pips >= (double)_triggering_delta)
                {
                    Order* pOrder = Order::make(SELL, (double)_stop_loss, (double)_take_profit, tick, (int)((double)_expriration_minutes));
                    assert(NULL != pOrder);
                    orders.push_back(pOrder);
                    pOrder->process_until_closing(current_tick_index);
                }

                _triggered_for_current_hour = true;
            }
        }

        virtual void process(const Tick& tick)
        {
            using namespace std;

            if((int)tick.timestamp().time_of_day().hours() != _current_hour)
            {
                _current_hour = (int)tick.timestamp().time_of_day().hours();
                _open_price = tick.bid();
                _triggered_for_current_hour = false;
                _high_for_the_hour = tick.bid();
            }
            else
            {
                if(tick.bid() > _high_for_the_hour)
                {
                    _high_for_the_hour = tick.bid();
                }

                if(_triggered_for_current_hour || (int) tick.timestamp().time_of_day().minutes() != (int)_minute_to_buy)
                {
                    return;
                }

                const double delta_in_pips = (_high_for_the_hour - _open_price) * 10000;

                if(delta_in_pips >= (double)_triggering_delta)
                {
                    add_order(Order::make(SELL, (double)_stop_loss, (double)_take_profit, tick, (int)((double)_expriration_minutes)));
                    _triggered_for_current_hour = true;
                }
            }
        }

        std::string get_full_description() const
        {
            std::string strg;
            strg += sformat("id:", "%20s");
            strg += sformat((int)_id, "%20d");
            strg += "\n";
            strg += sformat("minute to trade:", "%20s");
            strg += sformat((int)_minute_to_buy, "%20d");
            strg += "\n";
            strg += sformat("expriration minutes:", "%20s");
            strg += sformat((double)_expriration_minutes, "%20.5f");
            strg += "\n";
            strg += sformat("delta:", "%20s");
            strg += sformat((double)_triggering_delta, "%20.5f");
            strg += "\n";
            strg += sformat("stop loss:", "%20s");
            strg += sformat((double)_stop_loss, "%20.5f");
            strg += "\n";
            strg += sformat("profit take:", "%20s");
            strg += sformat((double)_take_profit, "%20.5f");
            strg += "\n";
            strg += sformat("number of orders:", "%20s");
            strg += sformat(orders_count(), "%20d");
            strg += "\n";
            strg += sformat("final balance:", "%20s");
            strg += sformat(get_account_balance(), "%20.2f");
            strg += "\n";
            strg += sformat("final pnl:", "%20s");
            strg += sformat(get_pnl(), "%20.2f");
            strg += "\n";
            strg += sformat("low:", "%20s");
            strg += sformat(get_absolute_low(), "%20.2f");
            strg += "\n";
            strg += sformat("winning trades:", "%20s");
            strg += sformat(get_winning_trades_count(), "%20d");
            strg += "\n";
            strg += sformat("losing trades:", "%20s");
            strg += sformat(get_loosing_trades_count(), "%20d");
            strg += "\n";
            strg += sformat("expired trades total:", "%20s");
            strg += sformat(get_expired_trades_count(), "%20d");
            strg += "\n";
            strg += sformat("winning expired count:", "%20s");
            strg += sformat(get_winning_expired_trades_count(), "%20d");
            strg += "\n";
            strg += sformat("losing expired count:", "%20s");
            strg += sformat(get_losing_expired_trades_count(), "%20d");
            strg += "\n";
            strg += sformat("max drawdown:", "%20s");
            strg += sformat(get_max_drawdown(), "%20.5f");
            strg += "\n";
            return strg;
        }


        std::string get_full_description2() const
        {
            std::string strg;
            strg += sformat("id:", "%20s");
            strg += sformat((int)_id, "%20d");
            strg += "\n";
            strg += sformat("minute to trade:", "%20s");
            strg += sformat((int)_minute_to_buy, "%20d");
            strg += "\n";
            strg += sformat("expriration minutes:", "%20s");
            strg += sformat((double)_expriration_minutes, "%20.5f");
            strg += "\n";
            strg += sformat("delta:", "%20s");
            strg += sformat((double)_triggering_delta, "%20.5f");
            strg += "\n";
            strg += sformat("stop loss:", "%20s");
            strg += sformat((double)_stop_loss, "%20.5f");
            strg += "\n";
            strg += sformat("profit take:", "%20s");
            strg += sformat((double)_take_profit, "%20.5f");
            strg += "\n";
            strg += _fitness_statistics.get_full_description();
            return strg;
        }

        static std::string printing_header()
        {
            std::string strg;
            strg += sformat("id", "%5s");
            strg += sformat("min.", "%10s");
            strg += sformat("delta", "%13s");
            strg += sformat("stoploss", "%13s");
            strg += sformat("profittake", "%13s");
            strg += sformat("#orders", "%10s");
            strg += sformat("fitness", "%20s");
            strg += sformat("PNL", "%20s");
            strg += sformat("drawdown", "%20s");
            strg += sformat("balance", "%20s");
            strg += sformat("abs_low", "%20s");
            strg += sformat("win_count", "%20s");
            strg += sformat("losse_count", "%20s");
            return strg;
        }

        virtual std::string to_string() const
        {
            std::string strg;
            strg += sformat(_id, "%5d");
            strg += sformat((int)_minute_to_buy, "%10d");
            strg += sformat((double)_triggering_delta, "%13.5f");
            strg += sformat((double)_stop_loss, "%13.5f");
            strg += sformat((double)_take_profit, "%13.5f");
            strg += sformat(orders_count(), "%10d");
            strg += " fitness: ";
            strg += sformat(get_fitness(), "%20.5f");
            strg += sformat(get_pnl(), "%20.5f");
            strg += sformat(get_max_drawdown() , "%20.5f");
            strg += sformat(get_account_balance(), "%20.5f");
            strg += sformat(get_absolute_low(), "%20.5f");
            strg += sformat(get_winning_trades_count(), "%20d");
            strg += sformat(get_loosing_trades_count(), "%20d");
            return strg;
        }
};

#endif

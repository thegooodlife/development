#ifndef TICK_ENGINE_INCLUDED
#define TICK_ENGINE_INCLUDED

///////////////////////////////////////////////////////
// TickEngine.h 
// 
// Author: John Pazarzis
//
// Date  : Sep. 28 2013
//
// Summary: 
//    Implements a tick simulator

////////////////////////////////////////////////////////////////////////


#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <time.h>
#include <functional>
#include "TickProcessor.h"
#include "Logger.h"



#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

#define BUFFER_LENGTH 3500
#define LINE_LENGTH 34
#define BUFFER_SIZE 100
#define CLEAR_BUFFER(buffer) for (int i = 0; i < BUFFER_LENGTH; ++i) buffer[i] = '\0';





class TickEngine
{
        std::vector<TickProcessor*> _processors;

        std::vector<TickProcessor*> _pending_to_register_processors;

        inline void register_pending_processors()
        {
            const int size = _pending_to_register_processors.size();
            for(register int i = 0; i < size; ++i)
                _processors.push_back(_pending_to_register_processors[i]);
            _pending_to_register_processors.clear();
        }

        inline void unregister_pending_processors()
        {
            std::vector<TickProcessor*>::iterator iter = _processors.begin();

            while(iter != _processors.end())
            {
                TickProcessor* p_tick_processor = (TickProcessor*) *iter;
                
                if(p_tick_processor->is_marked_to_stop_feed())
                {
                    iter = _processors.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
        }

        inline void update_pending_processors()
        {
            register_pending_processors();
            unregister_pending_processors();
        }

    public:

        void register_processor(TickProcessor* p_processor)
        {
            _pending_to_register_processors.push_back(p_processor);
        }   


        void run(const std::string& filename, long max_number_of_ticks = -1)
        {
            using namespace std;
            update_pending_processors();

            FILE* f = fopen (filename.c_str(), "r");
            if (f == NULL) 
            {
                LOG << "failed to open file: " << filename << EOL;
                throw("Error opening file");
            }

            char psz[BUFFER_LENGTH]; 
            CLEAR_BUFFER(psz)    
            int day, month, year, hour, minute, second;
            double bid, ask;
            int bytes_read;
            int counter = 0;
            Tick tick;
            long tick_count = 0;

            while ( (bytes_read =fread ( psz, LINE_LENGTH, BUFFER_SIZE, f)) > 0) 
            {
                for(register int i = 0; i < bytes_read; ++i)
                {
                    if(psz[i] == '\n') 
                        psz[i] = '\0';  
                }
                for(register int i = 0; i < BUFFER_SIZE; ++i)
                {
                    parse_tick((char*) &psz[i*LINE_LENGTH], tick);
                    tick_count += 1;
                    //if(tick_count % 1000000 == 0)
                    //   cout << "ticks so far: " << tick_count << endl;
                    update_pending_processors();
                    const int size = _processors.size();
                    for(register int i = 0; i < size; ++i)
                    {
                        _processors[i]->process(tick);
                      
                    }
                }
                CLEAR_BUFFER(psz)
                if (tick_count >= max_number_of_ticks && max_number_of_ticks > 0)
                {
                    break;
                }
            }      
            fclose (f);
        }

    private:

        static void parse_tick(char* psz, Tick& tick)
        {
            using namespace std;
            psz[2] = '\0';
            psz[5] = '\0';
            psz[8] = '\0';
            psz[11] = '\0';
            psz[14] = '\0';
            psz[17] = '\0';
            psz[25] = '\0';
            
            tick.day =  atoi(psz);
            tick.month = atoi((char*)&psz[3]);
            tick.year = atoi((char*)&psz[6]);
            tick.hour = atoi((char*)&psz[9]);
            tick.minute = atoi((char*)&psz[12]);
            tick.second = atoi((char*)&psz[15]);
            tick.bid = fast_atof((char*)&psz[18]);
            tick.ask = fast_atof((char*)&psz[26]);

        }

        static double fast_atof (const char *p)
        {
            int frac;
            double sign, value, scale;
            while (white_space(*p) ) 
            {
                p += 1;
            }
            sign = 1.0;
            if (*p == '-') 
            {
                sign = -1.0;
                p += 1;
            } 
            else if (*p == '+') 
            {
                p += 1;
            }
            for (value = 0.0; valid_digit(*p); p += 1) 
            {
                value = value * 10.0 + (*p - '0');
            }
            if (*p == '.') 
            {
                double pow10 = 10.0;
                p += 1;
                while (valid_digit(*p)) 
                {
                    value += (*p - '0') / pow10;
                    pow10 *= 10.0;
                    p += 1;
                }
            }

            frac = 0;
            scale = 1.0;
            if ((*p == 'e') || (*p == 'E')) {
                unsigned int expon;
                p += 1;
                if (*p == '-') {
                    frac = 1;
                    p += 1;

                } else if (*p == '+') {
                    p += 1;
                }

                for (expon = 0; valid_digit(*p); p += 1) {
                    expon = expon * 10 + (*p - '0');
                }
                if (expon > 308) expon = 308;
                while (expon >= 50) { scale *= 1E50; expon -= 50; }
                while (expon >=  8) { scale *= 1E8;  expon -=  8; }
                while (expon >   0) { scale *= 10.0; expon -=  1; }
            }
            return sign * (frac ? (value / scale) : (value * scale));
        }

};


#endif

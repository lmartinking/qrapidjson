/*
    qrapidjson - faster JSON serialiser extension for kdb+/q
    Copyright (C) 2016  Lucas Martin-King

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <cmath>
#include <ctime>
#include <arpa/inet.h> // for ntohl, etc

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#define KXVER 3
#include "k.h"

using namespace rapidjson;


template<typename Writer> void serialise_atom(Writer& w, K x, int i = -1);

template<typename Writer> void serialise_list(Writer& w, K x, bool isvec, int i = -1);

template<typename Writer> void serialise_sym(Writer& w, K x, bool isvec, int i = - 1);
template<typename Writer> void serialise_enum_sym(Writer& w, K x, bool isvec, int i = - 1);
template<typename Writer> void serialise_char(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_bool(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_byte(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_short(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_int(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_long(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_float(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_double(Writer& w, K x, bool isvec, int i = -1);

template<typename Writer> void serialise_date(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_time(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_timestamp(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_timespan(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_datetime(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_month(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_minute(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_second(Writer& w, K x, bool isvec, int i = -1);

template<typename Writer> void serialise_guid(Writer& w, K x, bool isvec, int i = -1);

template<typename Writer> void serialise_dict(Writer& w, K x, bool isvec, int i = -1);
template<typename Writer> void serialise_table(Writer& w, K x, bool isvec, int i = -1);

template<typename Writer> void serialise_keyed_table(Writer& w, K keys, K values);


template<typename Writer>
void serialise_list(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            K v = kK(x)[i];
            serialise_atom(w, v);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                K v = kK(x)[i];
                serialise_atom(w, v);
            }
            w.EndArray();
        }
    }
    else
    {
        std::cerr << "???";
        w.Null();
    }
}

template<typename Writer>
void serialise_sym(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            w.String((char*)kS(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                w.String((char*)kS(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        w.String(x->s);
    }
}

template<typename Writer>
void emit_enum_sym(Writer& w, K sym, int sym_idx)
{
    switch (sym_idx)
    {
        case(wi):
        case(ni):   w.Null(); break;
        default:    w.String((char*)kS(sym)[sym_idx]);
    }
}

template<typename Writer>
void serialise_enum_sym(Writer& w, K x, bool isvec, int i)
{
    K sym = k(0, (S)"sym", (K)0);
    if (! sym || sym->t != 11) {
        w.Null();
        return;
    }

    if (isvec)
    {
        if (i >= 0)
        {
            emit_enum_sym(w, sym, kI(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_enum_sym(w, sym, kI(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_enum_sym(w, sym, x->i);
    }
}

template<typename Writer>
void serialise_char(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        w.String((char*)&kC(x)[0], x->n);
    }
    else
    {
        w.String((char*)&x->g, 1);
    }
}

template<typename Writer>
void serialise_bool(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            w.Bool((int)kG(x)[i] != 0);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                w.Bool((int)kG(x)[i] != 0);
            }
            w.EndArray();
        }
    }
    else
    {
        w.Bool(x->g != 0);
    }
}

template<typename Writer>
inline void emit_byte(Writer& w, unsigned char n)
{
    const char * hex = "0123456789abcdef";
    char buff[2];

    buff[0] = hex[(n>>4) & 0xF];
    buff[1] = hex[ n     & 0xF];

    w.String(buff, 2);
}

template<typename Writer>
void serialise_byte(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_byte(w, kG(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_byte(w, kG(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_byte(w, x->g);
    }
}

template<typename Writer>
inline void emit_short(Writer& w, int n)
{
    switch (n)
    {
        case(wh):
        case(nh):   w.Null(); break;
        default:    w.Int(n); break;
    }
}

template<typename Writer>
void serialise_short(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_short(w, kH(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_short(w, kH(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_short(w, x->h);
    }
}

template<typename Writer>
inline void emit_int(Writer& w, int n)
{
    switch (n)
    {
        case(wi):
        case(ni):   w.Null(); break;
        default:    w.Int(n); break;
    }
}

template<typename Writer>
void serialise_int(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_int(w, kI(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_int(w, kI(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_int(w, x->i);
    }
}

template<typename Writer>
inline void emit_long(Writer& w, long long n)
{
    switch (n)
    {
        case(wj):
        case(nj):   w.Null(); break;
        default:    w.Int64(n); break;
    }
}

template<typename Writer>
void serialise_long(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_long(w, kJ(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_long(w, kJ(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_long(w, x->j);
    }
}

template<typename Writer>
inline void emit_double(Writer& w, double n)
{
    if (std::isnan(n))
    {
        w.Null();
    }
    else if (std::isinf(n))
    {
        w.String(n == INFINITY ? "Inf" : "-Inf");
    }
    else
    {
        w.Double(n);
    }
}

template<typename Writer>
void serialise_float(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_double(w, kE(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_double(w, kE(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_double(w, x->e);
    }
}

template<typename Writer>
void serialise_double(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_double(w, kF(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_double(w, kF(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_double(w, x->f);
    }
}

template<typename Writer>
inline void emit_date(Writer& w, int n)
{
    if (n == ni)
    {
        w.Null();
    }
    else
    {
        time_t tt = (n + 10957) * 8.64e4; // magic, see: https://github.com/kxcontrib/wiki/blob/master/csv.c
        struct tm timinfo;
        gmtime_r(&tt, &timinfo);
        char buff[10+1];
        snprintf(buff, sizeof(buff), "%04d-%02d-%02d", timinfo.tm_year+1900, timinfo.tm_mon+1, timinfo.tm_mday);
        w.String(buff, 10);
    }
}

template<typename Writer>
void serialise_date(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_date(w, kI(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_date(w, kI(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_date(w, x->i);
    }
}

template<typename Writer>
inline void emit_time(Writer& w, int n)
{
    if (n == ni)
    {
        w.Null();
    }
    else
    {
        time_t tt = n / 1000;
        struct tm timinfo;
        gmtime_r(&tt, &timinfo);
        char buff[12+1];
        snprintf(buff, sizeof(buff), "%02d:%02d:%02d.%03d", timinfo.tm_hour, timinfo.tm_min, timinfo.tm_sec, n % 1000);
        w.String(buff, 12);
    }
}

template<typename Writer>
void serialise_time(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_time(w, kI(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_time(w, kI(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_time(w, x->i);
    }
}

template<typename Writer>
inline void emit_timestamp(Writer& w, long long n)
{
    if (n == nj)
    {
        w.Null();
    }
    else
    {
        time_t tt = n * 1e-9 + 10957 * 8.64e4; // magic, see: https://github.com/kxcontrib/wiki/blob/master/csv.c
        struct tm timinfo;
        gmtime_r(&tt, &timinfo);
        char buff[29+1];
        snprintf(buff, sizeof(buff),
            "%04d-%02d-%02dD%02d:%02d:%02d.%09lld",
            timinfo.tm_year+1900, timinfo.tm_mon+1, timinfo.tm_mday,
            timinfo.tm_hour, timinfo.tm_min, timinfo.tm_sec, n%1000000000);
        w.String(buff, 29);
    }
}

template<typename Writer>
void serialise_timestamp(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_timestamp(w, kJ(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_timestamp(w, kJ(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_timestamp(w, x->j);
    }
}


template<typename Writer>
inline void emit_timespan(Writer& w, long long n)
{
    if (n == nj)
    {
        w.Null();
    }
    else
    {
        // FUDGE: tm_yday gets truncated to within (0, 365) so large timespans
        // do not convert correctly. Deletgate to kdb internal conversion instead.
        K x = kj(n);
        K sp = k(0, (S)"string `timespan$ ", x, (K)0);
        if (sp->t == KC)
        {
            w.String((char*)&kC(sp)[0], sp->n);
        }
        else
        {
            w.Null();
        }
        r0(sp);
    }
}

template<typename Writer>
void serialise_timespan(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_timespan(w, kJ(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_timespan(w, kJ(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_timespan(w, x->j);
    }
}

template<typename Writer>
inline void emit_datetime(Writer& w, double n)
{
    if (std::isnan(n))
    {
        w.Null();
    }
    else
    {
        time_t tt = (n + 10957) * 8.64e4;
        struct tm timinfo;
        gmtime_r(&tt, &timinfo);
        char buff[23+1];
        snprintf(buff, sizeof(buff),
            "%04d-%02d-%02dT%02d:%02d:%02d.%03lld",
            timinfo.tm_year+1900, timinfo.tm_mon+1, timinfo.tm_mday,
            timinfo.tm_hour, timinfo.tm_min, timinfo.tm_sec, (long long)(round(n*8.64e7))%1000);
        w.String(buff, 23);
    }
}

template<typename Writer>
void serialise_datetime(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_datetime(w, kF(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_datetime(w, kF(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_datetime(w, x->f);
    }
}

template<typename Writer>
inline void emit_month(Writer& w, const int n)
{
    if (n == ni)
    {
        w.Null();
    }
    else
    {
        int year = n/12+2000;
        int month = n%12+1;
        char buff[7 + 1];
        snprintf(buff, sizeof(buff), "%04d-%02d", year, month);
        w.String(buff, 7);
    }
}

template<typename Writer>
void serialise_month(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_month(w, kI(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_month(w, kI(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_month(w, x->i);
    }
}

template<typename Writer>
inline void emit_minute(Writer& w, const int n)
{
    if (n == ni)
    {
        w.Null();
    }
    else
    {
        time_t tt=n*60;
        struct tm timinfo;
        gmtime_r(&tt, &timinfo);
        char buff[5 + 1];
        snprintf(buff, sizeof(buff), "%02d:%02d", timinfo.tm_hour, timinfo.tm_min);
        w.String(buff, 5);
    }
}

template<typename Writer>
void serialise_minute(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_minute(w, kI(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_minute(w, kI(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_minute(w, x->i);
    }
}


template<typename Writer>
inline void emit_second(Writer& w, const int n)
{
    if (n == ni)
    {
        w.Null();
    }
    else
    {
        time_t tt=n*60;
        struct tm timinfo;
        gmtime_r(&tt, &timinfo);
        char buff[8 + 1];
        snprintf(buff, sizeof(buff), "%02d:%02d:%02d", timinfo.tm_hour, timinfo.tm_min, timinfo.tm_sec);
        w.String(buff, 8);
    }
}

template<typename Writer>
void serialise_second(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_second(w, kI(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_second(w, kI(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_second(w, x->i);
    }
}

template<typename Writer>
inline void emit_guid(Writer& w, const U guid_raw)
{
    static const U null_guid = {0};

    if (memcmp(&guid_raw, &null_guid, sizeof(U)) == 0)
    {
        w.Null();
    }
    else
    {
        union {
            struct {
                uint32_t data1;
                uint16_t data2;
                uint16_t data3;
                uint16_t data4;
                uint16_t data5;
                uint32_t data6;
            };
            U raw;
        } guid;

        guid.raw = guid_raw;
        char buff[36 + 1];
        snprintf(buff, sizeof(buff), "%08x-%04x-%04x-%04x-%04x%08x",
            ntohl(guid.data1), ntohs(guid.data2), ntohs(guid.data3), ntohs(guid.data4),
            ntohs(guid.data5), ntohl(guid.data6));
        w.String(buff, 36);
    }
}

template<typename Writer>
void serialise_guid(Writer& w, K x, bool isvec, int i)
{
    if (isvec)
    {
        if (i >= 0)
        {
            emit_guid(w, kU(x)[i]);
        }
        else
        {
            w.StartArray();
            for (int i = 0; i < x->n; i++)
            {
                emit_guid(w, kU(x)[i]);
            }
            w.EndArray();
        }
    }
    else
    {
        emit_guid(w, kU(x)[0]);
    }
}

template<typename Writer>
void serialise_dict(Writer& w, K x, bool isvec, int i)
{
    const K keys = kK(x)[0];
    const K values = kK(x)[1];

    #ifndef NDEBUG
    std::cerr << "Dict. key type:    " << (int)keys->t << std::endl;
    std::cerr << "Dict. key count:   " << keys->n << std::endl;
    std::cerr << "Dict. value count: " << values->n << std::endl;
    std::cerr << "Dict. value type:  " << (int)values->t << std::endl;
    #endif

    if (keys->t == XT && values->t == XT)
    {
        serialise_keyed_table(w, keys, values);
    }
    else
    {
        w.StartObject();
        for (int i = 0; i < keys->n; i++)
        {
            serialise_atom(w, keys, i);
            serialise_atom(w, values, i);
        }
        w.EndObject();
    }
}

template<typename Writer>
void serialise_keyed_table(Writer& w, K keys, K values)
{
    const K kdict = keys->k;
    const K kkeys = kK(kdict)[0];
    const K kvalues = kK(kdict)[1];

    const K vdict = values->k;
    const K vkeys = kK(vdict)[0];
    const K vvalues = kK(vdict)[1];

    const int krows = kK(kvalues)[0]->n;
    const int vrows = kK(vvalues)[0]->n;

    assert(vrows == krows);

    #ifndef NDEBUG
    std::cerr << "Keyed Table" << std::endl;
    std::cerr << "Key count:  " << kkeys->n << std::endl;
    std::cerr << "Key length: " << kvalues->n << std::endl;
    std::cerr << "Key rows:   " << krows << std::endl;

    std::cerr << "V column count: " << vkeys->n << std::endl;
    std::cerr << "V length:       " << vvalues->n << std::endl;
    std::cerr << "V rows:         " << vrows << std::endl;
    #endif

    // In kdb+, .j.j will serialise a keyed table as a dictionary of key objects to value objects.
    // However, this is not valid JSON. Instead, we serialise it as if it was an unkeyed table.
    w.StartArray();
    for (int i = 0; i < krows; i++)
    {
        w.StartObject();
        for (int j = 0; j < kkeys->n; j++)
        {
            serialise_atom(w, kkeys, j);
            serialise_atom(w, kK(kvalues)[j], i);
        }
        for (int j = 0; j < vkeys->n; j++)
        {
            serialise_atom(w, vkeys, j);
            serialise_atom(w, kK(vvalues)[j], i);
        }
        w.EndObject();
    }
    w.EndArray();
}

template<typename Writer>
void serialise_table(Writer& w, K x, bool isvec, int i)
{
    const K dict = x->k;
    const K keys = kK(dict)[0];
    const K values = kK(dict)[1];

    if (i >= 0)
    {
        w.StartObject();
        for (int j = 0; j < keys->n; j++)
        {
            serialise_atom(w, keys, j);
            serialise_atom(w, kK(values)[j], i);
        }
        w.EndObject();
    }
    else
    {
        const int rows = kK(values)[0]->n;

        w.StartArray();
        for (int i = 0; i < rows; i++)
        {
            w.StartObject();
            for (int j = 0; j < keys->n; j++)
            {
                serialise_atom(w, keys, j);
                serialise_atom(w, kK(values)[j], i);
            }
            w.EndObject();
        }
        w.EndArray();
    }
}

template<typename Writer>
void serialise_atom(Writer& w, const K x, int i)
{
    bool isvec = x->t >= 0;

    switch (x->t)
    {
        case (0):
            serialise_list(w, x, isvec, i); break;

        case (KS):
        case (-KS):
            serialise_sym(w, x, isvec, i); break;

        case (KC):
        case (-KC):
            serialise_char(w, x, isvec, i); break;

        case (KB):
        case (-KB):
            serialise_bool(w, x, isvec, i); break;

        case (KG):
        case (-KG):
            serialise_byte(w, x, isvec, i); break;

        case (KH):
        case (-KH):
            serialise_short(w, x, isvec, i); break;

        case (KI):
        case (-KI):
            serialise_int(w, x, isvec, i); break;

        case (KJ):
        case (-KJ):
            serialise_long(w, x, isvec, i); break;

        case (KE):
        case (-KE):
            serialise_float(w, x, isvec, i); break;

        case (KF):
        case (-KF):
            serialise_double(w, x, isvec, i); break;

        case (XT):
            serialise_table(w, x, isvec, i); break;

        case (XD):
            serialise_dict(w, x, isvec, i); break;

        case (KD):
        case (-KD):
            serialise_date(w, x, isvec, i); break;

        case (KT):
        case (-KT):
            serialise_time(w, x, isvec, i); break;

        case (KP):
        case (-KP):
            serialise_timestamp(w, x, isvec, i); break;

        case (KZ):
        case (-KZ):
            serialise_datetime(w, x, isvec, i); break;

        case (UU):
        case (-UU):
            serialise_guid(w, x, isvec, i); break;

        case (KM):
        case (-KM):
            serialise_month(w, x, isvec, i); break;

        case (KN):
        case (-KN):
            serialise_timespan(w, x, isvec, i); break;

        case (KU):
        case (-KU):
            serialise_minute(w, x, isvec, i); break;

        case (KV):
        case (-KV):
            serialise_second(w, x, isvec, i); break;

        // MAGIC: Enumerated symbols (eg: splayed tables)
        case (20):
        case (-20):
            serialise_enum_sym(w, x, isvec, i); break;

        default:
            #ifndef NDEBUG
            std::cerr << "WARNING: unhandled atom (" << (int)x->t << ")" << std::endl;
            #endif
            w.Null();
            break;
    }
}

extern "C" K tojson(K x)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    serialise_atom(writer, x);

    size_t len = buffer.GetLength();
    const char* str = buffer.GetString();

    // Marshal to kdb
    K ser = kpn((char*)str, len);

    return ser;
}

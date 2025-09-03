
assert: {[condition;message]
  if[not condition; 'message];
  -1 message,"... OK";
  };

tojson: (`$"qrapidjson_l64") 2:(`tojson;1);

////////////////////////////////////////////
///////////  Type Speed Test ///////////////
////////////////////////////////////////////

-1 "---- .j.j vs qrapidjson speed test -----";
-1 " ";
-1 "Running Test...";

t: ([] sym: enlist `Symbol;
       str: enlist "String";
       float: enlist 1.23456789;
       int: enlist 12345678;
       date: 2000.01.01;
       datetime:.z.Z;
       timestamp:.z.P;
       month:2025.01m;
       minute:12:56;
       second: 23:33:33;
       time: 23:33:33.567;
       dict: (enlist (`a`b`c!(1 2 3)))
   );

timeJsonFormat:{[d]
    cppTimes:raze {[d;c] stTime:.z.P; tojson ?[d;();0b;enlist[c]!enlist c]; enlist[c]!enlist .z.P - stTime}[d;] each cols d;
    jjTimes:raze {[d;c] stTime:.z.P; .j.j ?[d;();0b;enlist[c]!enlist c]; enlist[c]!enlist .z.P - stTime}[d;] each cols d;
    res:([]columns:key cppTimes; jjTimes: value jjTimes; cppTimes: value cppTimes);
    update percentageDiff:100* cppTimes % jjTimes from res
 };

result: timeJsonFormat 1000000#t;
-1 " ";
-1 {[row] "|" sv {[val] ((25 - count[val])#" "),val} each "," vs row} each "," 0: result;

////////  Example output  ////////

// columns   jjTimes              cppTimes             percentageDiff
// ------------------------------------------------------------------
// sym       0D00:00:00.437909861 0D00:00:00.057969381 13.23774
// str       0D00:00:00.383909687 0D00:00:00.056136581 14.62234
// float     0D00:00:00.713113690 0D00:00:00.090411623 12.67843
// int       0D00:00:00.516855134 0D00:00:00.038151248 7.38142
// date      0D00:00:00.456810763 0D00:00:00.075890479 16.61311
// datetime  0D00:00:00.497317994 0D00:00:00.112435518 22.60838
// timestamp 0D00:00:00.530782453 0D00:00:00.555663386 104.6876
// month     0D00:00:00.458526562 0D00:00:00.207092080 45.16469
// minute    0D00:00:00.415678540 0D00:00:00.218988129 52.68209
// second    0D00:00:00.402389783 0D00:00:00.279551047 69.4727
// time      0D00:00:00.432864537 0D00:00:00.306168906 70.73088
// dict      0D00:00:01.881094694 0D00:00:00.152304691 8.096599


////////////////////////////////////////////
///////////  Exactness Test ///////////////
////////////////////////////////////////////
-1 " ";
-1 "Checking if the values in the .j.j serialised table match the values in the qrapidjson serialised table";
-1 " ";

tab:([] int:100000?1000000i;
        float:100000?1000f;
        sym:100000?`8;
        str:string 100000?`8;
        char: first each string 100000?`1;
        long:100000?1000000;
        date:100000?2101.01.01;
        datetime:"Z"$string 100000?2101.01.01T00:00:00.000;
        timestamp:100000?2101.01.01D00:00:00.000000000
    );

jjTab: .j.k .j.j tab;
// .j.j serialises timestamp in datetime format
jjTab:update {ssr[x;"T";"D"]} each timestamp from jjTab;
cppTab: .j.k tojson tab;

floatResult: all {x within (y - 0.001; y + 0.001)}'[exec float from jjTab; exec float from cppTab];
assert[floatResult;"test floats are ~ equivalent"];

allResult: (delete float from jjTab) ~ delete float from cppTab;
assert[allResult;"test all non float columns are matching"];


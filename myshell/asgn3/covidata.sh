#!/bin/bash
# Name: Yani Gong
# Student ID: 260907830

errorMsg() {
    echo "Error: $1"
    echo "Script syntax:"
    echo "./covidata.sh -r procedure id range inputFile outputFile compareFile"
    echo "Legal usage examples:"
    # args num 4
    echo "./covidata.sh get 35 data.csv result.csv"
    # args num 7
    echo "./covidata.sh -r get 35 2020-01 2020-03 data.csv result.csv"
    # args num 5
    echo "./covidata.sh compare 10 data.csv result2.csv result.csv"
    # args num 8
    echo "./covidata.sh -r compare 10 2020-01 2020-03 data.csv result2.csv result.csv"
}


if [[ "$1" == "-r" ]]; then
    if [[ "$2" != "compare" ]] && [[ "$2" != "get" ]]; then
        errorMsg "Procedure not provided"
        exit
    fi
    if [[ "$2" == "compare" ]] && [[ "$#" -ne 8 ]]; then
        errorMsg "Wrong number of arguments"
        exit
    fi
    if [[ "$2" == "get" ]] && [[ "$#" -ne 7 ]]; then
        errorMsg "Wrong number of arguments"
        exit
    fi
elif [[ "$1" != "compare" ]] && [[ "$1" != "get" ]]; then
    errorMsg "Procedure not provided"
    exit
fi

if [[ "$1" == "compare" ]] && [[ "$#" -ne 5 ]]; then
    errorMsg "Wrong number of arguments"
    exit
fi
if [[ "$1" == "get" ]] && [[ "$#" -ne 4 ]]; then
    errorMsg "Wrong number of arguments"
    exit
fi

if [[ "$1" != "-r" ]] && [[ "$1" == "get" ]]; then
    # awk -F, '$1==35{print $0}' data.csv
    if [[ -r "$3" ]]; then
        awk -F, '$1=='$2'{print $0}' "$3" > "$4"
        rowcount=`grep -c "" "$4"`
        if [[ $rowcount == 0 ]]; then
            echo "Procedure ID error"
            exit
        fi
        # echo $rowcount
        avgconf=`cat $4 | awk -F, '{sum+=$6;} END {printf("%.2f", sum/NR)}'`
        # echo $avgconf
        avgdeaths=`cat $4 | awk -F, '{sum+=$8;} END {printf("%.2f", sum/NR)}'`
        # echo $avgdeaths
        avgtests=`cat $4 | awk -F, '{sum+=$11;} END {printf("%.2f", sum/NR)}'`
        # echo $avgtests
        echo "rowcount,avgconf,avgdeaths,avgtests" >> "$4"
        echo $rowcount,$avgconf,$avgdeaths,$avgtests >> "$4"
    else
        errorMsg "Input file name does not exist"
        exit
    fi
fi

#  "./covidata.sh compare id inputFile outputFile compareFile"
#  "./covidata.sh compare 10 data.csv result2.csv result.csv"
if [[ "$1" != "-r" ]] && [[ "$1" == "compare" ]]; then
    if [[ -r "$3" ]] && [[ -r "$5" ]]; then
        awk -F, '$1=='$2'{print $0}' "$3" > "$4"
        rowcount=`grep -c "" "$4"`
        avgconf=`cat $4 | awk -F, '{sum+=$6;} END {printf("%.2f", sum/NR)}'`
        avgdeaths=`cat $4 | awk -F, '{sum+=$8;} END {printf("%.2f", sum/NR)}'`
        avgtests=`cat $4 | awk -F, '{sum+=$11;} END {printf("%.2f", sum/NR)}'`
        cat $5 >> $4
        compare=`tail -n 1 $4`
        sed -i '$d' $4

        echo $rowcount,$avgconf,$avgdeaths,$avgtests >> "$4"
        echo $compare >> "$4"
        echo "diffcount,diffavgconf,diffavgdeath,diffavgtests" >> "$4"
        cmp_rowcount=`echo $compare | awk -F, 'END {print $1}'`
        cmp_avgconf=`echo $compare | awk -F, 'END {print $2}'`
        cmp_avgdeaths=`echo $compare | awk -F, 'END {print $3}'`
        cmp_avgtests=`echo $compare | awk -F, 'END {print $4}'`

        diffcount=`echo "scale=2; $rowcount-$cmp_rowcount" | bc`
        diffavgconf=`echo "scale=2; $avgconf-$cmp_avgconf" | bc`
        diffavgdeath=`echo "scale=2; $avgdeaths-$cmp_avgdeaths" | bc`
        diffavgtests=`echo "scale=2; $avgtests-$cmp_avgtests" | bc`
        echo $diffcount,$diffavgconf,$diffavgdeath,$diffavgtests >> "$4"
        # echo `'{printf(".2f", cmp_rowcount)}'`
    else
        errorMsg "Input file name does not exist"
        exit
    fi
fi

# ./covidata.sh -r get 35 2020-01 2020-03 data.csv result.csv
# "./covidata.sh -r procedure id range inputFile outputFile compareFile"
# awk -F, -v d="10/01/2021" '$4~d' data.csv
if [[ "$1" == "-r" ]] && [[ "$2" == "get" ]]; then
    # awk -F, '$1==35{print $0}' data.csv
    if [[ -r "$6" ]]; then
        awk -F, '$1=='$3'{print $0}' "$6" > "$7"
        result=`awk -F, '$1=='$3'{print $0}' "$6"`
        echo "rowcount,avgconf,avgdeaths,avgtests" >> "$7"
        # echo "$result"
        # result=`echo "$result" | awk -F, -v d="2020-1-31" '$5~d'`
        # echo "$result"
        startDate=$4
        startYear=${startDate:0:4}
        startMonth=${startDate:5:6}

        endDate=$5
        endYear=${endDate:0:4}
        endMonth=${endDate:5:6}

        startDay="01"
        endDay="15"
        while [ $startYear -le $endYear ]; do
            if [ $startYear != $endYear ]; then
                currentEndMonth="12"
            else
                currentEndMonth=$endMonth
            fi
                while [ $startMonth -le $currentEndMonth ]; do
                    # debug for testing
                    # echo $startDay, $endDay, $startMonth,$currentEndMonth, $theNewStartDate, $theNewEndDate, $monthVal

                    # AWK or SED
                    theNewStartDate=$startYear-$startMonth-$startDay
                    theNewEndDate=$startYear-$startMonth-$endDay

                    dayVal="$startDay"
                    resultByDate=""
                    while [ $dayVal -le $endDay ]; do
                        # monthVal=`echo $startMonth`
                        # dayVal=`echo $dayVal`
                        monthVal=`echo $startMonth`
                        dayVal=`echo $dayVal`
                        if [ ${#monthVal} -lt 2 ]; then
                            monthVal=0$monthVal
                        fi
                        if [ ${#dayVal} -lt 2 ]; then
                            dayVal=0$dayVal
                        fi
                        # echo "$startYear-$monthVal-$dayVal"
                        # tmp=`echo "$result" | awk -F, -v d="$startYear-$monthVal-$dayVal" '$5~d'`
                        # cat data.csv | awk -F, -v d="$A" '$5==d{print $0}'
                        tmp=`echo "$result" | awk -F, -v d="$startYear-$monthVal-$dayVal" '$5==d{print $0}'`
                        if [ -n "$tmp" ]; then
                            resultByDate+="$tmp"$'\n'
                        fi
                        dayVal=$(expr $dayVal + 1)
                    done
                    # echo "$resultByDate"

                    rowcount=`echo "$resultByDate" | awk -F, 'END {print NR}'`
                    # echo $rowcount
                    if [[ $rowcount == 0 ]]; then
                        echo "Procedure ID or date error"
                        exit
                    fi
                    avgconf=`echo "$resultByDate" | awk -F, '{sum+=$6;} END {printf("%.2f", sum/NR)}'`
                    # echo $avgconf
                    avgdeaths=`echo "$resultByDate" | awk -F, '{sum+=$8;} END {printf("%.2f", sum/NR)}'`
                    # echo $avgdeaths
                    avgtests=`echo "$resultByDate" | awk -F, '{sum+=$11;} END {printf("%.2f", sum/NR)}'`
                    # # echo $avgtests
                    # echo "rowcount,avgconf,avgdeaths,avgtests" >> "$7"
                    echo $rowcount,$avgconf,$avgdeaths,$avgtests >> "$7"
                    if [ $startDay -eq "01" ]; then
                        startDay="16"
                        endDay="31"
                    elif [ $startDay -eq "16" ]; then
                        startDay="01"
                        endDay="15"
                        startMonth=$(expr $startMonth + 1)
                        if [ ${#startMonth} -lt 2 ]; then
                            startMonth=0$startMonth
                        fi
                    fi
                done
                startYear=$(expr $startYear + 1)
                startMonth="01"
        done
    else
        errorMsg "Input file name does not exist"
        exit
    fi
fi

# "./covidata.sh -r compare 10 2020-01 2020-03 data.csv result2.csv result.csv"
# "./covidata.sh -r procedure id range inputFile outputFile compareFile"
if [[ "$1" == "-r" ]] && [[ "$2" == "compare" ]]; then
    if [[ -r "$6" ]] && [[ -r "$8" ]]; then
        awk -F, '$1=='$3'{print $0}' "$6" > "$7"
        result_input=`awk -F, '$1=='$3'{print $0}' "$6"`
        result_compare=`cat $8`
        cat $8 >> $7
        compare=`tail -n 1 $7`
        while [ "$compare" != "rowcount,avgconf,avgdeaths,avgtests" ]; do
            sed -i '$d' $7
            compare=`tail -n 1 $7`
        done

        startDate=$4
        startYear=${startDate:0:4}
        startMonth=${startDate:5:6}

        endDate=$5
        endYear=${endDate:0:4}
        endMonth=${endDate:5:6}

        startDay="01"
        endDay="15"
        index="0"
        while [ $startYear -le $endYear ]; do
            if [ $startYear != $endYear ]; then
                currentEndMonth="12"
            else
                currentEndMonth=$endMonth
            fi
                while [ $startMonth -le $currentEndMonth ]; do
                    # AWK or SED
                    theNewStartDate=$startYear-$startMonth-$startDay
                    theNewEndDate=$startYear-$startMonth-$endDay

                    dayVal="$startDay"
                    resultByDate=""
                    while [ $dayVal -le $endDay ]; do
                        monthVal=`echo $startMonth`
                        # dayVal=`echo $dayVal`
                        if [ ${#monthVal} -lt 2 ]; then
                            monthVal=0$monthVal
                        fi
                        if [ ${#dayVal} -lt 2 ]; then
                            dayVal=0$dayVal
                        fi
                        tmp=`echo "$result_input" | awk -F, -v d="$startYear-$monthVal-$dayVal" '$5==d{print $0}'`
                        if [ -n "$tmp" ]; then
                            resultByDate+="$tmp"$'\n'
                        fi
                        dayVal=$(expr $dayVal + 1)
                    done

                    rowcount=`echo "$resultByDate" | awk -F, 'END {print NR}'`
                    # echo $rowcount
                    if [[ $rowcount == 0 ]]; then
                        echo "Procedure ID or date error"
                        exit
                    fi
                    avgconf=`echo "$resultByDate" | awk -F, '{sum+=$6;} END {printf("%.2f", sum/NR)}'`
                    # echo $avgconf
                    avgdeaths=`echo "$resultByDate" | awk -F, '{sum+=$8;} END {printf("%.2f", sum/NR)}'`
                    # echo $avgdeaths
                    avgtests=`echo "$resultByDate" | awk -F, '{sum+=$11;} END {printf("%.2f", sum/NR)}'`
                    # # echo $avgtests

                    avgrowcount_input[index]=$rowcount
                    avgconf_input[index]=$avgconf
                    avgdeaths_input[index]=$avgdeaths
                    avgtests_input[index]=$avgtests
                    # echo ${avgconf_input[index]}
                    index=$(expr $index + 1)

                    echo $rowcount,$avgconf,$avgdeaths,$avgtests >> "$7"

                    if [ $startDay -eq "01" ]; then
                        startDay="16"
                        endDay="31"
                    elif [ $startDay -eq "16" ]; then
                        startDay="01"
                        endDay="15"
                        startMonth=$(expr $startMonth + 1)
                        if [ ${#startMonth} -lt 2 ]; then
                            startMonth=0$startMonth
                        fi
                    fi
                done
                startYear=$(expr $startYear + 1)
                startMonth="01"
        done
        echo "rowcount,avgconf,avgdeaths,avgtests" >> "$7"

        startDate=$4
        startYear=${startDate:0:4}
        startMonth=${startDate:5:6}

        endDate=$5
        endYear=${endDate:0:4}
        endMonth=${endDate:5:6}

        startDay="01"
        endDay="15"
        index="0"
        while [ $startYear -le $endYear ]; do
            if [ $startYear != $endYear ]; then
                currentEndMonth="12"
            else
                currentEndMonth=$endMonth
            fi
                while [ $startMonth -le $currentEndMonth ]; do
                    # AWK or SED
                    theNewStartDate=$startYear-$startMonth-$startDay
                    theNewEndDate=$startYear-$startMonth-$endDay

                    dayVal="$startDay"
                    resultByDate=""
                    while [ $dayVal -le $endDay ]; do
                        monthVal=`echo $startMonth`
                        dayVal=`echo $dayVal`
                        if [ ${#monthVal} -lt 2 ]; then
                            monthVal=0$monthVal
                        fi
                        if [ ${#dayVal} -lt 2 ]; then
                            dayVal=0$dayVal
                        fi
                        tmp=`echo "$result_compare" | awk -F, -v d="$startYear-$monthVal-$dayVal" '$5==d{print $0}'`
                        if [ -n "$tmp" ]; then
                            resultByDate+="$tmp"$'\n'
                        fi
                        dayVal=$(expr $dayVal + 1)
                    done

                    rowcount=`echo "$resultByDate" | awk -F, 'END {print NR}'`
                    # echo $rowcount
                    if [[ $rowcount == 0 ]]; then
                        echo "Procedure ID or date error"
                        exit
                    fi
                    avgconf=`echo "$resultByDate" | awk -F, '{sum+=$6;} END {printf("%.2f", sum/NR)}'`
                    # echo $avgconf
                    avgdeaths=`echo "$resultByDate" | awk -F, '{sum+=$8;} END {printf("%.2f", sum/NR)}'`
                    # echo $avgdeaths
                    avgtests=`echo "$resultByDate" | awk -F, '{sum+=$11;} END {printf("%.2f", sum/NR)}'`
                    # echo $avgtests
                    echo $rowcount,$avgconf,$avgdeaths,$avgtests >> "$7"

                    avgrowcount_compare[index]=$rowcount
                    avgconf_compare[index]=$avgconf
                    avgdeaths_compare[index]=$avgdeaths
                    avgtests_compare[index]=$avgtests
                    # echo ${avgconf_compare[index]}
                    index=$(expr $index + 1)

                    if [ $startDay -eq "01" ]; then
                        startDay="16"
                        endDay="31"
                    elif [ $startDay -eq "16" ]; then
                        startDay="01"
                        endDay="15"
                        startMonth=$(expr $startMonth + 1)
                        if [ ${#startMonth} -lt 2 ]; then
                            startMonth=0$startMonth
                        fi
                    fi
                done
                startYear=$(expr $startYear + 1)
                startMonth="01"
        done
    echo "diffcount,diffavgconf,diffavgdeath,diffavgtests" >> "$7"

    for (( i=0; i<$index; i=i+1 )); do
        diffcount=`echo "scale=2; ${avgrowcount_input[i]}-${avgrowcount_compare[i]}" | bc`
        diffavgconf=`echo "scale=2; ${avgconf_input[i]}-${avgconf_compare[i]}" | bc`
        diffavgdeath=`echo "scale=2; ${avgdeaths_input[i]}-${avgdeaths_compare[i]}" | bc`
        diffavgtests=`echo "scale=2; ${avgtests_input[i]}-${avgtests_compare[i]}" | bc`
        echo $diffcount,$diffavgconf,$diffavgdeath,$diffavgtests >> "$7"
    done
    else
        errorMsg "Input file name does not exist"
        exit
    fi
fi

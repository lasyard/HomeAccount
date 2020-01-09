# README

HomeAccount is an application to record and calculate personal financial data.  The datum are saved in encrypted form but can be exported to plain text files.

## Functions of buttons

* Monthly: show the monthly datum.  The calendar can be used to change the date.
* Cash: show the cash distribution.
* Total: show the total report of income and outlay, classified by categories, years/months.
* Export: export data to text file.
* Import: import data from text file.

## Cash Distribution

You can edit a text file and import it as cash file.  Here is an example of the text file:

```
#Deposit
#Debt
#Credit card
```

The lines beginning with "#" are not calculated but displayed.

## Categories

You can edit a text file and import it as category config.  Here is an example of the text file:

```
###Income
##Regular
#Salary
###Outlay
##Food
##Clothes
##Men's
##Women's
```

The lines beginning with 3 "#"s are first-level class; those with 2 "#"s are second-level class and those with 1 "#" are third-level class.  After the "#"s is the name of the class.  No duplicate names are allowed even they belong to different upper-level class.  The first first-level class is always regarded as "income" and the last first-level class is always regarded as "outlay".  No more first-level classes are needed.

If you set a class for an item with a specified description, the classification will be stored. When you write the same description for another item next time, the class will be automatically set.

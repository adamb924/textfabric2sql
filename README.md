# textfabric2sql
The excellent [BHSA data](https://etcbc.github.io/bhsa/) provides a morphological analysis of the entire Old Testament. The original data are in TextFabric format (a plain text file format). That format has been converted to MySQL for their website ([SHEBANQ](https://shebanq.ancient-data.org/)), but these SQL tables do not include all of the features. This code converts the TextFabric data into SQL format—either SQLite or MySQL.

## Just the data
If you just want the data, it's in the `data` folder. `bhsa2021.sqlite.gz` is a gziped SQLite database. `bhsa2021.sql.gz` is a gzipped *dump* of a MySQL database.

## Running the program
You're welcome to run `textfabric2sql` yourself. It takes three parameters:

1. The path to the folder containing the `.tf` files. If you get this wrong you will see an error about not being able to read `otype.tf`.
2. The format you want: either `mysql` or `sqlite`.
3. A connection string. For `sqlite` this is just a filename. For `mysql`, this is a string like the one shown below, giving the hostname, database name, and all of that.

Here is a sample MySQL run:
```
./textfabric2sql "C:\Users\Adam\bhsa\tf\2021" mysql "hostname=localhost;databasename=bhsa2021;username=myusername;password=mypassword"
```
(Note that the MySQL database needs to exist already. For the above run you would need to have run: `CREATE DATABASE bhsa2021;`)

Here is a sample SQLite run:
```
./textfabric2sql "C:\Users\Adam\bhsa\tf\2021" sqlite "bhsa2021.sqlite"
```

For reasons that I haven't been able to figure out, it takes much longer to execute with MySQL than with  SQLite. I welcome any feedback on the code.

## Technical Notes

### 1. Row and Column Display Order

 The TFT Display and the datasheet have some discrepecies. In the datasheet, the MADCTL register has the D7 and D6 bits handling the Row and Column Order. Those two bits decide how the frame buffer will reflect on the display.

- D7 (Row): 0 -> Top to Bottom, 1-> Bottom to Top
- D6 (Column): 0 -> Left to Right, 1-> Right to Left

Now the problem is every display has different configurations depending on the manufacturer and mine has two changes in the bits we talked about.

1) The bits to change the Row and Column Order have been swapped. So, D7 is for Column and D6 for Row.

2) The values in those bits correspond to the opposite of what is in the datasheet.

Essentially, according to the datasheet, D7 controls the Row Order with 0 as top to bottom and 1 as bottom to top. On my display, the D7 bit changes the column order and to add to that the bit values are 0 for right to left and 1 for left to right. Similarly for D6, in the datasheet it is resposible for column order but on my display it changes the row order with 0 for bottom to top and 1 for top to bottom.

In short, it took a lot of time to debug.
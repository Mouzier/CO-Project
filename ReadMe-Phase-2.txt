
The restriction on the input is same as before.

The only changes are the choices of the user.

	1. The user is asked the name of the file.
	2. The user is asked whether they want data forwarding or not. (1 - For data forwarding, 0 - for no data forwarding)

Branch predictor is not considered. Whenever there is a branch instruction in RF stage, the next instruction is fetched. After the RF stage, we will get the correct PC value and will fetch the correct instruction. (IF happens twice consecutively irrespective of the outcome).
So there will be no stalls due to branch instructions but number of cycles will increase due to IF being done twice.

Whenever we get (RF - stall - RF), it is considered as (stall - stall - RF).
Whenever we get (RF - RF), it is considered as (stall - RF).
We are counting an extra stall in both cases.


Output will be following:
	
	1. The instructions where the stall occurs along with its number of stalls
	2. Number of cycles
	3. Number of stalls
	4. Number of instructions
	5. IPC
	6. The values of the 32 register and those of the memory elements



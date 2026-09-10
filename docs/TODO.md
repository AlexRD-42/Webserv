1. Ensure all fields being stored in parsing are no longer than 4096 bytes	(Done?)

2. Possibly make s_length_check print the error line

4. Validate the CGI interpreters (check for access)

5. Do more target validation inside check_location, it is too permissive

7. Make multi-level bitarray (see how to do specializations)

9. Maybe reduce the allocation alignment to 16 bytes, cache locality might be hurt

10. Configure signal handling

13. Change timeout design

14. Verify the close on exec

16. Verify exit strategies for PERR_EXIT

17. Create the file handling for query target and read file

18. When upload store is not present, convert it to URI

19. Change bitfields to a bitmap

20. Error pages as a folder
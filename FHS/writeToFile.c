#include <stdio.h> 
#include <string.h> 
   
int main( ) 
{ 
  
    FILE *fptr ; 
       
    char textToBeWritten[50]  = "Master 2 LSE 2019"; 
  
    fptr = fopen("test.txt", "a") ; 
      
    // Check if this filePointer is null 
    // which maybe if the file does not exist 
    if ( fptr == NULL ) 
    { 
        printf( "failed to open test.txt." ) ; 
    } 
    else
    { 
               
        if ( strlen (  textToBeWritten  ) > 0 ) 
        { 
              
            // writing in the file using fputs() 
            fputs(textToBeWritten, fptr) ;    
            fputs("\n", fptr) ; 
        } 
          
        // Closing the file using fclose() 
        fclose(fptr) ; 
          
        printf("Text successfully written \n"); 
    } 
    return 0;         
} 

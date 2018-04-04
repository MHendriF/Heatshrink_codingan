int main(int argc, char **argv)
{
    init(); // this is needed

    Serial.begin( 9600 );

    for ( ;; )
        Serial.println( millis() );
}

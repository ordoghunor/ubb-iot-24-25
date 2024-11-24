import { useState, useRef } from "react";
import mqtt from "mqtt";
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';
import Form from 'react-bootstrap/Form';
import '../../styles/CustomSwitch.css';

export default function LightSensor() {
    const [isConnected, setIsConnected] = useState(false);
    const [isChecked, setIsChecked] = useState(false);

    const topic = 'home/kitchen/humidity'
    const clientRef = useRef(null);

    const handleSwitchConnection = () => {
        if(!isConnected) 
        {
            clientRef.current = mqtt.connect('mqtt://localhost:9001');
            clientRef.current.on('connect', function () {
                setIsConnected(true);
                console.log(`connected dehumidifier`);
            });

            clientRef.current.subscribe(topic, function () {
                console.log(`dehumidifier subscribed to ${topic}'`);
           });
   
           clientRef.current.on('message', function (receivedTopic, mess) {
               if (receivedTopic === topic) {
                   console.log(`Dehumidifier received: ${mess} on ${receivedTopic}`);
                   const messObject = JSON.parse(mess);
                   setIsChecked(messObject.value === 0);
               }
           });
        } else {
            if(clientRef.current) {
                clientRef.current.unsubscribe(topic, function () {
                    console.log(`Dehumidifier: unsubscribed from topic: ${topic}`);
                });
                console.log(`deconnected dehumidifier`);
                clientRef.current.end(); // Close the MQTT connection
                clientRef.current = null;
            }
            setIsConnected(false);
        }
    }

    return (
        <>
            <Row>
                <h2>Dehumidifier</h2>
            </Row>
            <Row>
            <div className="form-check-container">
                    <h4>Connection status:</h4> 
                    <Form.Check
                    type="switch"
                    id="custom-switch"
                    checked={isConnected}
                    className="checker"
                    onChange={handleSwitchConnection}
                    />
                </div>
            </Row> 
            <Row className='button_row'>
                <Col className='button_col'>
                    Is running:
                </Col>
                <Col className='button_col'>
                    <Form.Check
                    type="switch"
                    id="custom-switch"
                    checked={isChecked}
                    readOnly
                    className="checker"
                    />
                </Col>
            </Row>
        </>
    );
}

import { useEffect, useState } from "react";
import mqtt from "mqtt";
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';
import Button from 'react-bootstrap/Button';

export default function LightSensor() {
    const [isConnected, setIsConnected] = useState(false);
    const [isActiveSensor, setIsActiveSensor] = useState();
    const client = mqtt.connect('mqtt://localhost:9001');
    const topic = 'home/kitchen/humidity'

    useEffect(() => {
        client.on('connect', function () {
            setIsConnected(true);
            console.log('connected light sensor');
        });

        return () => {
        //   client.unsubscribe('light', function () {
        //       console.log('unsubscribed from light');
        //   });
          client.end(); // Close the MQTT connection
      };

    }, []);

    const handleClick = (message) => {
        setIsActiveSensor(message);
        const data = JSON.stringify({"status": message});
        client.publish(topic, data);
    }

    return (
        <>
            <Row>
                <h2>Humidity sensor</h2>
            </Row>
            <Row>
                <h4>ConnectionStatus: {isConnected ? 'Connected' : 'Connecting...'}</h4>
            </Row>
            <Row>
                <h4>Active status: {isActiveSensor === 'ON' ? 'Active' : 'Not active'}</h4>
            </Row>
            <Row className='button_row'>
                <Col className='button_col'>
                    <Button className='switch_button' onClick={() => handleClick('ON')}>ON</Button>  
                </Col>
                <Col className='button_col'>
                    <Button className='switch_button' onClick={() => handleClick('OFF')}>OFF</Button>
                </Col>
            </Row>
        </>
    );
}

import { useState, useRef } from "react";
import mqtt from "mqtt";
import Row from 'react-bootstrap/Row';
import Form from 'react-bootstrap/Form';
import DayNightToggle from 'react-day-and-night-toggle'

export default function LightSensor() {
    const [isConnected, setIsConnected] = useState(false);
    const [isDarkened, setIsDarkened] = useState(false);
    const topic = 'home/outdoor/light'
    const clientRef = useRef(null);

    const handleClick = () => {
        setIsDarkened(!isDarkened);
        if(!clientRef.current) return;

        if (!isDarkened) {
            const data = JSON.stringify({"value": 100});
            clientRef.current.publish(topic, data);
            return;
        }
        const data = JSON.stringify({"value": 10});
        clientRef.current.publish(topic, data);
    }

    const handleSwitchConnection = () => {
        if(!isConnected) 
        {
            clientRef.current = mqtt.connect('mqtt://localhost:9001');
            clientRef.current.on('connect', function () {
                setIsConnected(true);
                console.log(`connected light sensor`);
            });
        } else {
            if(clientRef.current) {
                console.log(`deconnected light sensor`);
                clientRef.current.end(); // Close the MQTT connection
                clientRef.current = null;
            }
            setIsConnected(false);
        }
    }

    return (
        <>
            <Row>
                <h2>Light sensor</h2>
            </Row>
            <Row>
                <div className="form-check-container">
                    <h4>ConnectionStatus:</h4>
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
            <div style={{
            display: 'flex',
            justifyContent: 'center',
            alignItems: 'center',
            }}>
                <DayNightToggle 
                    onChange={handleClick}
                    checked={isDarkened}
                    className='my-2'
                />
            </div>
            </Row>
        </>
    );
}

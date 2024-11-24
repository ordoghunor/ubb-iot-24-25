import mqtt from "mqtt";
import { useEffect, useState, useRef  } from "react";
import Row from "react-bootstrap/Row";
import ProgressBar from 'react-bootstrap/ProgressBar';
import Form from 'react-bootstrap/Form';

export default function SmokeSensor({ topic, location }) {
    const [isConnected, setIsConnected] = useState(false);
    const [progressValue, setProgressValue] = useState(0);
    const [increasing, setIncreasing] = useState(true);
    const clientRef = useRef(null);

    useEffect(() => {
        const interval = setInterval(() => {
            setProgressValue((prevProgress) => {
                if(!isConnected) return 0; 
                if (increasing) {
                    if(clientRef.current)
                        clientRef.current.publish(topic, JSON.stringify({ "value": prevProgress + 3 }), {qos: 1, retain: true});
                    if (prevProgress >= 60) {
                        setIncreasing(false);
                        return prevProgress - 3;
                    }
                    return prevProgress + 3;
                } else {
                    if(clientRef.current)
                        clientRef.current.publish(topic, JSON.stringify({ "value": prevProgress - 3 }), {qos: 1, retain: true});
                    if (prevProgress <= 0) {
                        setIncreasing(true);
                        return prevProgress + 3;
                    }
                    return prevProgress - 3;
                }
            });
        }, 1000);

        // Clean up the interval on component unmount
        return () => clearInterval(interval);
    }, [increasing, isConnected]);

    const handleSwitchConnection = () => {
        if(!isConnected) 
        {
            clientRef.current = mqtt.connect('mqtt://localhost:9001');
            clientRef.current.on('connect', function () {
                setIsConnected(true);
                console.log(`connected ${location} smoke sensor`);
            });
        } else {
            if(clientRef.current) {
                console.log(`deconnected ${location} smoke sensor`);
                clientRef.current.end(); // Close the MQTT connection
                clientRef.current = null;
            }
            setIsConnected(false);
        }
    }

    return (
        <>
            <Row>
                <h2>Smoke sensor</h2>
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
            <Row>
                <h4>Location: {location}</h4>
            </Row>
            <Row className='button_row'>
                <ProgressBar striped variant="danger" now={progressValue} label={`${progressValue}%`} />
            </Row>
        </>
    );
}

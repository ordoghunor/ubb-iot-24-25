import Container from 'react-bootstrap/Container';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';
import LightSensor from './Sensors/LightSensor';
import SmokeSensor from './Sensors/SmokeSensor';
import Dehumidifier from './Devices/Dehumidifier'
import SteamExtractor from './Devices/SteamExtractor'
import '../styles/Home.css';
import ProblemDetector from './Devices/ProblemDetector';

export default function Home() {
    return (
        <Container className='grid_system'>
            <Row>
                <Col className='dashboard_col'>
                    <SmokeSensor topic='home/living/smoke' location='living' />
                </Col>
                <Col className='dashboard_col'>
                    <SmokeSensor topic='home/kitchen/smoke' location='kitchen' />
                </Col>
            </Row>
            <Row>
                <Col className='dashboard_col'>
                    <SteamExtractor topic='home/living/smoke' location='living' />
                </Col>
                <Col className='dashboard_col'>
                    <SteamExtractor topic='home/kitchen/smoke' location='kitchen' />
                </Col>
            </Row>
            <Row>
                <Col className='dashboard_col'>
                    <LightSensor />
                </Col>
                <Col className='dashboard_col'>
                    <Dehumidifier />
                </Col>
            </Row>
            <Row>
                <Col className='dashboard_col'>
                    <ProblemDetector />
                </Col>
            </Row>
        </Container>
    );
}

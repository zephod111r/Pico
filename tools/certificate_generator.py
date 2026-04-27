from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives.serialization import Encoding, PrivateFormat, NoEncryption
from datetime import datetime, timedelta

# Generate private key
private_key = rsa.generate_private_key(
    public_exponent=65537,
    key_size=2048,
)

# Generate self-signed certificate
subject = issuer = x509.Name([
    x509.NameAttribute(NameOID.COUNTRY_NAME, u"UK"),
    x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, u"Kent"),
    x509.NameAttribute(NameOID.LOCALITY_NAME, u"Folkestone"),
    x509.NameAttribute(NameOID.ORGANIZATION_NAME, u"My Company"),
    x509.NameAttribute(NameOID.COMMON_NAME, u"localhost"),
])
cert = x509.CertificateBuilder().subject_name(
    subject
).issuer_name(
    issuer
).public_key(
    private_key.public_key()
).serial_number(
    x509.random_serial_number()
).not_valid_before(
    datetime.utcnow()
).not_valid_after(
    datetime.utcnow() + timedelta(days=365)
).sign(private_key, hashes.SHA256())

# Write the private key to a file
with open("private.key", "wb") as f:
    f.write(private_key.private_bytes(
        encoding=Encoding.PEM,
        format=PrivateFormat.TraditionalOpenSSL,
        encryption_algorithm=NoEncryption(),
    ))

# Write the certificate to a file
with open("selfsigned.crt", "wb") as f:
    f.write(cert.public_bytes(Encoding.PEM))